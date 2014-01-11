class NodeId(object):
    def __init__(self, node_id):
        self.node_id = node_id

    def __str__(self):
        return str(self.node_id)

    def __repr__(self):
        return str(self)

    def __eq__(self, other):
        if other is None:
            return False

        if self.node_id is None or other.node_id is None:
            return False

        if self.node_id[0] == "*" or \
           other.node_id[0]=="*":
            return self.node_id[1] == other.node_id[1]

        return self.node_id == other.node_id
