__author__ = 'Christopher Nelson'

from collections import Counter
import logging

class Election(object):
    """
    Maintains state about, and performs a leader election. Once the election is over
    this object can go away.
    """
    def __init__(self, nodes, election_started, election_duration=10):
        self.votes = {node_id : 0 for node_id in nodes}
        self.voters = set()
        self.nodes = nodes
        self.election_started = election_started
        self.election_duration = election_duration

    def __unicode__(self):
        return "Election (%d voters, %d nodes)" % (len(self.voters), len(self.nodes))

    def __repr__(self):
        return self.__unicode__()

    def ready(self, current_node_time):
        """
        Indicates if the election is ready to proceed.

        :param current_node_time: The current node time.
        :return:
        """
        if not self.election_has_concluded(current_node_time):
            return False

        return self.has_majority()

    def undecideable(self, current_node_time):
        """
        Indicates if the election is undecideable. This occurs when the election duration period has
        passed and there are not enough voters to decide anything.

        :return: True if the election is undecideable, False otherwise
        """
        if not self.election_has_concluded(current_node_time):
            return False

        return not self.has_majority()


    def tally(self, vote_for, vote_from):
        """
        Tallies a vote from 'vote_from' and for 'vote_for'. Multiple votes are not allowed. Once a majority is
        achieved, the election is over.

        :param vote_for: The node id to vote for.
        :param vote_from: The node id the vote is from.
        :return: None
        """
        vote_from = tuple(vote_from)
        vote_for = tuple(vote_for)

        # No duplicate votes
        if vote_from in self.voters:
            logging.warning("Duplicate vote from node: %s ignored.", vote_from)
            return

        # No unregistered voters
        if vote_from not in self.nodes:
            logging.error("Vote from unknown node: %s", vote_from)
            return

        # Tally a vote
        current_vote = self.votes.get(vote_for, 0)
        self.votes[vote_for] = current_vote+1
        self.voters.add(vote_from)

    def has_majority(self):
        """
        Indicates if we have reached a majority of voters.
        :return: True if we have a majority, False otherwise.
        """
        if len(self.voters) <= (len(self.nodes)/2):
            return False

        c = Counter(self.votes)
        mc = c.most_common(2)

        # We requested the two most common votes. If we only got one then it
        # means that everyone voted for the same node. Easy out.
        if len(mc)==1:
            return True

        # Now we need to check if the two most common items are equal, or if one
        # of the candidates has a majority.
        if mc[0][1] > mc[1][1]:
            return True

        # We have not reached a majority.
        return False

    def election_has_concluded(self, current_node_time):
        """
        Indicates if the election has concluded, meaning the time for the election has elapsed.
        :param current_node_time: The current node time.
        :return: True if the election has concluded, False otherwise.
        """
        return not (current_node_time - self.election_started <= self.election_duration)

    def get_best_candidate(self):
        """
        Determines the best candidate to vote for. This has nothing to do with the current vote tally, but instead
        is just a simple algorithm which should allow all nodes to converge on the new leader.
        :return: The node id of the best candidate.
        """
        return max(self.nodes)

    def get_winner(self):
        """
        Finds the winner of the election. In order for the results of this function to be valid
        you must first check that the election has a majority.
        :return: The node id of the winning candidate.
        """
        c = Counter(self.votes)
        mc = c.most_common(1)
        return mc[0][0]

