#!/usr/bin/env python3

from pyonionreq.consensus import direct
from pyonionreq.onionmaker import all_aesgcm as aesgcm


def test_aesgcm_onion_maker():

    net = direct()

    path = net.make_path_selector()
    fetcher = net.make_node_fetcher()

    fetcher.FetchAll(lambda nodes : path.set_node_list(nodes))
    
    maker = aesgcm()
