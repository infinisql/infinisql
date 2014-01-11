__author__ = 'Christopher Nelson'
__version__ = 0.1

def main():
    import argparse
    import logging

    from infinisqlmgr import config, data_server, management_server

    parser = argparse.ArgumentParser(prog="isqlmgr", description='Manage an InfiniSQL cluster.')

    parser.add_argument('--dist-dir', dest='dist_dir',
                       default="/opt/infinisql",
                       help='The path where the database distribution is installed. (default is %(default)s)')

    parser.add_argument('--host', dest='management_host',
                       default="localhost",
                       help='The management host to control. (default is %(default)s)')

    parser.add_argument('--port', dest='management_port',
                       default=21000,
                       help='The management port. (default is %(default)s)')

    parser.add_argument('--version', action='version',
                       version="%(prog)s " + str(__version__))

    parser.add_argument('--debug', dest='debug', action='store_true',
                       default=False,
                       help='Enable debug logging. (default is off)')

    sp = parser.add_subparsers()
    management_server.add_args(sp)
    data_server.add_args(sp)

    # Process the arguments
    args = parser.parse_args()
    configuration = config.Configuration(args)

    # The func attribute is set by parse_args() and represents the command handler
    # for whatever sub-command was specified by the user.
    if hasattr(args, "func"):
        args.func(configuration)


if __name__ == "__main__":
    main()
