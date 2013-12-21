__author__ = 'Christopher Nelson'

import configparser
import getpass
import fcntl
import socket
import struct
import os

import infinisqlmgr.getifaddrs

class Configuration(object):
    def __init__(self, args):
        #############################################
        # Configuration file and distribution
        self.args = args
        self.dist_dir = self._get("installation", "dist_dir", "/opt/infinisql")

        self.config = configparser.ConfigParser(interpolation=configparser.ExtendedInterpolation())
        self.config.read(["infinisql.conf",
                          "etc/infinisql.conf",
                          os.path.join(self.dist_dir, "etc", "infinisql.conf")])

        #############################################
        # Network settings for management components.

        mt = "management"
        if not self.config.has_section(mt):
            self.config.add_section(mt)

        self.set_default(mt, "management_interface", "*")
        self.set_default(mt, "management_ip", self.resolve(mt, "management_interface"))
        self.set_default(mt, "management_port", 21000)
        self.set_default(mt, "management_host", "localhost")

        self.set_default(mt, "management_node_id", "%s:%s" % (self.config.get(mt, "management_ip"),
                                                              self.config.get(mt, "management_port")))

        self.set_default(mt, "announcement_mcast_group", "224.0.0.1")
        self.set_default(mt, "announcement_mcast_port", 21001)

        self.set_default(mt, "configuration_interface", "*")
        self.set_default(mt, "configuration_ip", self.resolve(mt, "configuration_interface"))
        self.set_default(mt, "configuration_port", 21002)

        ##########################################
        # Cluster configuration for management

        self.set_default(mt, "cluster_name", "default_cluster")
        self.set_default(mt, "log_config_file", os.path.join(self.dist_dir, "etc", "${cluster_name}.logging.conf"))
        self.set_default(mt, "log_file", os.path.join(self.dist_dir, "var", "log", "${cluster_name}.management.log"))
        self.set_default(mt, "debug", False)

        ##########################################
        # Metrics configuration

        mc = "metrics"
        if not self.config.has_section(mc):
            self.config.add_section(mc)

        self.set_default(mc, "data_dir", os.path.join(self.dist_dir, "var"))

        ##########################################
        # Data Engine Configuration

        de = "data engine"
        if not self.config.has_section(de):
            self.config.add_section(de)

        self.set_default(de, "infinisqld", os.path.join(self.dist_dir, "sbin", "infinisqld"))
        self.set_default(de, "log_file", os.path.join(self.dist_dir, "var", "log", "infinisqld.log"))
        self.set_default(de, "global_admin_password", "passw0rd")

        self.set_default(de, "configuration_interface", "*")
        self.set_default(de, "configuration_ip", self.resolve(de, "configuration_interface",
                                                              "engine_configuration_interface"))
        self.set_default(de, "configuration_port", 11520)
        self.set_default(de, "legacy_listen_interface", "*")
        self.set_default(de, "legacy_listen_ip", self.resolve(de, "legacy_listen_interface"))
        self.set_default(de, "legacy_listen_port", 11521)
        self.set_default(de, "inbound_gateway_port", 11530)
        self.set_default(de, "sql_interface", "*")
        self.set_default(de, "sql_ip", self.resolve(de, "sql_interface"))
        self.set_default(de, "sql_port", 15432)

        self.set_default(de, "transaction_agent_count", 6)
        self.set_default(de, "engine_count", 6)
        self.set_default(de, "inbound_gateway_count", 1)
        self.set_default(de, "outbound_gateway_count", 1)
        self.set_default(de, "anonymous_ping", 1)
        self.set_default(de, "bad_login_messages", 1)

        self.set_default(de, "user_schema_manager_node_id", 1)
        self.set_default(de, "deadlock_manager_node_id", 1)
        self.set_default(de, "active_replica", 0)

        self.set_default(de, "replica", 0)
        self.set_default(de, "member", 0)

        ##########################################
        # Installation Configuration

        il = "installation"
        if not self.config.has_section(il):
            self.config.add_section(il)

        self.set_default(il, "user_name", getpass.getuser())
        self.set_default(il, "ssh_key_file", os.path.join(os.environ["HOME"], ".ssh", "id_dsa"))
        self.set_default(il, "install_path", "/opt/infinisql")
        self.set_default(il, "push_list", None)

    def _get(self, section, name, default):
        """
        Tries to fetch the argument value from the args object if it exists, otherwise returns default.
        :param name: The argument to fetch.
        :param default: The value to return if argument doesn't exist.
        :return: The value or default.
        """
        if hasattr(self.args, name):
            return getattr(self.args, name)
        if hasattr(self, "config") and self.config.has_option(section, name):
            return self.config.get(section, name, default)
        return default

    def get(self, section, name):
        """
        Forward to the configuration object.
        :param section: the section to look in.
        :param name: The option to fetch.
        :param default: The value to return if option doesn't exist.
        :return: The value or default.
        """
        return self.config.get(section, name)

    def get_int(self, section, name):
        """
        Forward to the configuration object.
        :param section: the section to look in.
        :param name: The option to fetch.
        :param default: The value to return if option doesn't exist.
        :return: The value or default.
        """
        return self.config.getint(section, name)

    def get_boolean(self, section, name):
        """
        Forward to the configuration object.
        :param section: the section to look in.
        :param name: The option to fetch.
        :param default: The value to return if option doesn't exist.
        :return: The value or default.
        """
        return self.config.getboolean(section, name)

    def set_default(self, section, name, default):
        """
        Sets the default value of the option at section:name.
        :param section: The section to write to.
        :param name: The name to write to.
        :param default: The default value to provide if "name" is not in args.
        :return: None
        """
        if hasattr(self.args, name):
            self.config.set(section, name, str(getattr(self.args, name)))
        self.config.set(section, name, str(default))

    def resolve(self, section, interface, args=None):
        """
        Resolves an ip address from an interface configuration. If the interface is "*" then all interfaces are
        listened on. Otherwise we find the IP for the specified interface and use that.

        :param section: The section to look in for the interface key.
        :param interface: The interface key.
        :param args: The interface key for command-line arguments if different.
        :return: The IP address to listen on.
        """
        args = args if args is not None else interface
        if hasattr(self.args, args):
            value = getattr(self.args, args)
        elif self.config.has_option(section, interface):
            value = self.config.get(section, interface)
        else:
            value = "*"

        return "*" if value == "*" else self.ip(interface=value)

    def ip(self, interface="eth0"):
        """
        :param interface: The interface to get an ip address for.
        :return: A string containing the ip address.
        """

        interfaces = infinisqlmgr.getifaddrs.getifaddrs()
         

