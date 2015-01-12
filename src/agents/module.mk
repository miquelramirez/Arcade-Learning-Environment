MODULE := src/agents

MODULE_OBJS := \
	src/agents/bit_array.o \
	src/agents/PlayerAgent.o \
	src/agents/RandomAgent.o \
	src/agents/SingleActionAgent.o \
	src/agents/SDLKeyboardAgent.o \
	src/agents/SimpleBanditAgent.o \
	src/agents/SearchAgent.o \
	src/agents/SearchTree.o \
	src/agents/TreeNode.o \
	src/agents/FullSearchTree.o \
	src/agents/UCTSearchTree.o \
	src/agents/UCTTreeNode.o \
	src/agents/UCTNoveltySearchTree.o \
	src/agents/UCTNoveltyTreeNode.o \
	src/agents/BreadthFirstSearch.o \
	src/agents/IW1Search.o \
	src/agents/IW1DijkstraSearch.o \
	src/agents/BestFirstSearch.o \
	src/agents/UniformCostSearch.o \
	src/agents/IW2Search.o \
	src/agents/Width2BestFirstSearch.o \
	src/agents/IW11Search.o \
	src/agents/BFS_UCB.o

MODULE_DIRS += \
	src/agents

# Include common rules 
include $(srcdir)/common.rules
