__author__ = 'Christopher Nelson'

import logging
import os
import signal

from infinisqlmgr import common, management

def start_management_server(args):
    from infinisqlmgr.management import util

    common.configure_logging(args)
    existing_pid = util.get_pid(args.dist_dir, args.cluster_name)
    if existing_pid is not None:
        logging.error("A management process appears to exist already. You should run the 'manager stop' command first "
                      "to make sure the existing process has stopped.")
        return 1

    logging.debug("forking management server")
    pid = os.fork()

    if pid!=0:
        util.write_pid(args.dist_dir, args.cluster_name, pid)
        logging.info("Parent start_management_server() finished")
        return 0

    logging.debug("creating management process")
    management_server = management.Controller(args.cluster_name)
    logging.debug("starting management process")
    return management_server.run()


def stop_management_server(args):
    from infinisqlmgr.management import util

    common.configure_logging(args)

    existing_pid = util.get_pid(args.dist_dir, args.cluster_name)
    if existing_pid is not None:
        logging.info("Trying to stop the existing process at pid %d", existing_pid)
        try:
            os.kill(existing_pid, signal.SIGTERM)
        except ProcessLookupError:
            logging.debug("the management process is not running")
        else:
            logging.info("Waiting for process %d exit", existing_pid)
            try:
                pid, exit_status = os.waitpid(existing_pid, 0)
            except ChildProcessError:
                # We ignore this because the child process might have already gone away, and we
                # won't be able to get status information about it.
                pass
            else:
                return_code = exit_status >> 8
                logging.debug("management process exited with code %d", return_code)
                if return_code!=0:
                    logging.warning("There was an error while stopping the management process, check the logs for more detail.")

    # Make sure that the pid file is gone, even if it's empty.
    if util.exists(args.dist_dir, args.cluster_name):
        run_path = util.get_run_path(args.dist_dir, args.cluster_name)
        logging.debug("deleting run file at: %s", run_path)
        os.unlink(run_path)

    logging.info("Stopped management process for cluster: %s" % args.cluster_name)


def add_args(sub_parsers):
    mgr_parser = sub_parsers.add_parser('manager', help='Options for controlling a management process')
    mgr_parser.add_argument('--no-background', dest='daemonize', action='store_false',
                              default=True,
                              help='Do not run the manager in the background. Useful for debugging. (default is off)')
    mgr_parser.add_argument('--cluster-name', dest='cluster_name',
                              default="default_cluster",
                              help='Set the cluster name to join. If the cluster does not exist it will be created. '
                                   '(default is %(default)s)')

    ss_parsers = mgr_parser.add_subparsers()
    start_parser = ss_parsers.add_parser('start', help='Start a management process')
    start_parser.set_defaults(func=start_management_server)

    stop_parser = ss_parsers.add_parser('stop', help='Stop a management process')
    stop_parser.set_defaults(func=stop_management_server)

