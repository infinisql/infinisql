__author__ = 'Christopher Nelson'

import logging

def configure_logging(args):
    log_format = "%(asctime)-15s %(levelname)s (%(module)s:%(lineno)s) %(message)s"
    logging.basicConfig(level=logging.INFO if not args.debug else logging.DEBUG, format=log_format)

