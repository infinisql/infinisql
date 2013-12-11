__author__ = 'Christopher Nelson'

import logging
import os


def get_run_path(dist_dir, cluster_name):
    """
    Provides the path to the run file.
    :param dist_dir: The distribution root dir.
    :param cluster_name: The name of the cluster to look for.
    :return: An absolute, normalized path to the run file.
    """
    return os.path.abspath(os.path.normpath(os.path.join(dist_dir, "run", "%s.management.pid" % cluster_name)))


def exists(dist_dir, cluster_name):
    """
    Determines if the run file for the management process exists.

    :param dist_dir: The distribution root dir.
    :param cluster_name: The name of the cluster to look for.
    :return: True if the management.pid file exists, False otherwise.
    """
    run_path = get_run_path(dist_dir, cluster_name)
    return os.path.exists(run_path)


def get_pid(dist_dir, cluster_name):
    """
    Provides the pid of the management process.

    :param dist_dir: The distribution root dir.
    :param cluster_name: The name of the cluster to look for.
    :return: None if there is no pid, otherwise the pid as an integer.
    """
    if not exists(dist_dir, cluster_name):
        return None

    run_path = get_run_path(dist_dir, cluster_name)
    with open(run_path, "r") as run_file:
        return int(run_file.read().strip())


def write_pid(dist_dir, cluster_name, pid):
    """
    Writes (or overwrites) the pid for the management process of the given cluster.

    :param dist_dir: The distribution root dir.
    :param cluster_name: The name of the cluster to look for.
    :param pid: The pid to write.
    """
    run_path = get_run_path(dist_dir, cluster_name)
    run_dir = os.path.dirname(run_path)
    if not os.path.exists(run_dir):
        logging.info("Creating run path: %s", run_dir)
        os.makedirs(run_dir)

    logging.debug("writing pid %d into %s", pid, run_path)
    with open(run_path, "w") as run_file:
        run_file.write(str(pid))
