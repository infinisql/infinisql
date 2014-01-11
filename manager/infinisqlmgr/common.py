__author__ = 'Christopher Nelson'

import logging
import os

from io import StringIO

def configure_logging(config):
    from logging import handlers

    debug = config.get_boolean("management", "debug")
    log_conf = config.get("management", "log_config_file")
    if os.path.exists(log_conf):
        # Configure logging from a file.
        logging.config.fileConfig(log_conf)
        # Make sure that we have actually configured something in the file.
        if logging.getLogger().hasHandlers():
            # Override all handlers with DEBUG mode if that's set on the command line
            if config.getboolean("debug"):
                for handler in logging.getLogger().handlers:
                    handler.setLevel(logging.DEBUG)
            return

    # Perform default configuration
    log_format = "%(asctime)-15s %(levelname)s (%(module)s:%(lineno)s) %(message)s"
    log_path = config.get("management", "log_file")
    log_dir = os.path.dirname(log_path)
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)

    logger = logging.getLogger()
    formatter = logging.Formatter(log_format)

    # create console handler
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO if not debug else logging.DEBUG)
    ch.setFormatter(formatter)

    # create file handler
    fh = handlers.RotatingFileHandler(log_path, maxBytes=1024*1024, backupCount=10)
    fh.setLevel(logging.INFO if not debug else logging.DEBUG)
    fh.setFormatter(formatter)

    # add handlers
    logger.addHandler(ch)
    logger.addHandler(fh)

    # write actual configuration to log file
    buf = StringIO()
    config.config.write(buf)
    logging.debug(buf.getvalue())

