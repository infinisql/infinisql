__author__ = 'Christopher Nelson'

from infinisqlmgr import common

def start_management_server(args):
    common.configure_logging(args)

def stop_management_server(args):
    common.configure_logging(args)


def add_args(sub_parsers):
    mgr_parser = sub_parsers.add_parser('manager', help='Options for controlling a management process')
    mgr_parser.add_argument('--no-daemon', dest='daemonize', action='store_false',
                              default=True,
                              help='Do not daemonize the manager. Useful for debugging. (default is off)')

    ss_parsers = mgr_parser.add_subparsers()
    start_parser = ss_parsers.add_parser('start', help='Start a management processs')
    start_parser.set_defaults(func=start_management_server)

    stop_parser = ss_parsers.add_parser('stop', help='Stop a management processs')
    stop_parser.set_defaults(func=stop_management_server)

