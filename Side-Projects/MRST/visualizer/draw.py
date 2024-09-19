import matplotlib.pyplot as plt
import sys

# ********************************
# *          Read file           *
# ********************************
with open(sys.argv[1], 'r') as fin:
    lines = fin.readlines()

nodeNum = int(lines[0].strip())
edgeNum = int(lines[nodeNum+2].strip())
# print(f"# of nodes: {nodeNum}")

node_positions = []
node_dict = {}
for i in range(1, nodeNum+1):
    seg = lines[i].strip().split()
    if(len(seg) == 3):
        nodeID, x, y = map(int, seg)
        node_positions.append((nodeID, x, y))
    else:
        nodeID, x, y = map(int, seg[:3])
        isSteiner = True
        node_positions.append((nodeID, x, y, isSteiner))
    node_dict[nodeID] = (x,y)
# print(f"node coordinates: {node_positions}")

edge_start = nodeNum + 3
edges = []
for i in range(edge_start, edge_start + edgeNum):
    edge = tuple(map(int, lines[i].strip().split()))
    edges.append(edge)
# print(f"edges: {edges}")


# *******************************
# *         Plot graph          *
# *******************************
fig, ax = plt.subplots()

# plot nodes
for pos in node_positions:
    if(len(pos) == 3):
        nodeID, x, y = pos
        ax.scatter(x, y, s=200, color='skyblue')  # plot circle
        ax.text(x, y, str(nodeID), fontsize=8, ha='center', va='center', color='black')  # print nodeID in the middle of circle

# plot edges
for edge in edges:
    start_node_id, end_node_id = edge
    x_start, y_start = node_dict[start_node_id]
    x_end, y_end = node_dict[end_node_id]
    ax.plot([x_start, x_end], [y_start, y_end], color='gray', linewidth=2)

# set axis
ax.set_xlabel('X-axis')
ax.set_ylabel('Y-axis')
ax.tick_params(left=True, bottom=True, labelleft=True, labelbottom=True)

ax.set_xlim(0, 100)
ax.set_ylim(0, 100)

ax.spines['left'].set_position('zero')  
ax.spines['bottom'].set_position('zero')
ax.spines['right'].set_color('none')     
ax.spines['top'].set_color('none')       

plt.grid(True)

plt.show()