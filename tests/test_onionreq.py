#!/usr/bin/env python3

from pyonionreq.consensus import Direct
from pyonionreq.onionmaker import all_aesgcm as aesgcm


def set_nodes(path, nodes):
    print("setting node list with {} nodes".format(len(nodes.keys())))
    path.set_node_list(nodes)

def test_aesgcm_onion_maker():
    return
    net = Direct(["curve://public.loki.foundation:22027/3c157ed3c675f56280dc5d8b2f00b327b5865c127bf2c6c42becc3ca73d9132b"])

    path = net.make_path_selector()
    fetcher = net.make_node_fetcher()
    print("bootstrapping")

    fetcher.fetch_all(lambda nodes : set_nodes(path, nodes))
