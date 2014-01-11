__author__ = 'Christopher Nelson'

import logging
import os
import signal

from infinisqlmgr import common, management

def start_data_server(args):
    pass

def stop_data_server(args):
    pass

def show_data_server(args):
    pass

def list_data_servers(args):
    pass

def add_args(sub_parsers):
    mgr_parser = sub_parsers.add_parser('dbe', help='Options for controlling database engine processes')
    mgr_parser.add_argument('--node', dest='node_id',
                              default=None,
                              help='The node to control.')

    ss_parsers = mgr_parser.add_subparsers()
    start_parser = ss_parsers.add_parser('start', help='Start a data engine process')
    start_parser.add_argument('--config-file', dest='config_file',
                              default=None,
                              help='Sets the configuration file to use to configure the database engine.'
                                   '(default is %(default)s)')
    start_parser.set_defaults(func=start_data_server)

    stop_parser = ss_parsers.add_parser('stop', help='Stop a data engine process')
    stop_parser.set_defaults(func=stop_data_server)

    list_parser = ss_parsers.add_parser('list', help='List active data engine processes')
    list_parser.set_defaults(func=list_data_servers)

    show_parser = ss_parsers.add_parser('show', help='Show information about a data engine process')
    show_parser.set_defaults(func=show_data_server)

