__author__ = 'Christopher Nelson'

import logging
import os

def configure_logging(args):
    from logging import handlers

    log_conf = os.path.join(args.dist_dir, "etc", "%s.logging.conf" % args.cluster_name)
    if os.path.exists(log_conf):
        # Configure logging from a file.
        logging.config.fileConfig(log_conf)
        # Make sure that we have actually configured something in the file.
        if logging.getLogger().hasHandlers():
            # Override all handlers with DEBUG mode if that's set on the command line
            if args.debug:
                for handler in logging.getLogger().handlers:
                    handler.setLevel(logging.DEBUG)
            return

    # Perform default configuration
    log_format = "%(asctime)-15s %(levelname)s (%(module)s:%(lineno)s) %(message)s"
    log_path = os.path.abspath(os.path.normpath(os.path.join(args.dist_dir, "log", "%s.log" % args.cluster_name)))
    log_dir = os.path.dirname(log_path)
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)

    logger = logging.getLogger()
    formatter = logging.Formatter(log_format)

    # create console handler
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO if not args.debug else logging.DEBUG)
    ch.setFormatter(formatter)

    # create file handler
    fh = handlers.RotatingFileHandler(log_path, maxBytes=1024*1024, backupCount=10)
    fh.setLevel(logging.INFO if not args.debug else logging.DEBUG)
    fh.setFormatter(formatter)

    # add handlers
    logger.addHandler(ch)
    logger.addHandler(fh)

