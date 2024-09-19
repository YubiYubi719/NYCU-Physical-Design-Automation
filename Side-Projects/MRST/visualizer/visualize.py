import pygame
import json
import glob
import time
import re
import sys
import Util

pygame.init()

screen = pygame.display.set_mode((Util.WINDOW_WIDTH, Util.WINDOW_HEIGHT))
pygame.display.set_caption('Graph Animation')

def load_graph_from_json(file):
    with open(file, 'r') as f:
        data = json.load(f)
    return data

def draw_grid():
    screen.fill(Util.WHITE)
    
    width, height = screen.get_size()
    
    # plot grid
    for x in range(0, width, Util.GRID_SPACING):
        pygame.draw.line(screen, Util.GRAY, (x, 0), (x, height), 1)  # vertical
    for y in range(0, height, Util.GRID_SPACING):
        pygame.draw.line(screen, Util.GRAY, (0, y), (width, y), 1)  # horizontal

def draw_graph(data):
    draw_grid()

    nodes = data['nodes']
    edges = data['edges']

    # plot edges
    for edge in edges:
        start_node = edge['start']
        end_node = edge['end']
        
        start_pos = get_node_position(start_node, nodes)
        end_pos = get_node_position(end_node, nodes)
        
        if start_pos and end_pos:
            pygame.draw.line(screen, Util.EDGE_COLOR, start_pos, end_pos, 5)

    # plot nodes
    for node in nodes:
        scaled_pos = (int(node['x'] * Util.SCALE_FACTOR), Util.WINDOW_HEIGHT - int(node['y'] * Util.SCALE_FACTOR))

        if node.get('isSteiner', False):
            # plot steiner nodes by small and pink circle
            pygame.draw.circle(screen, Util.STEINER_COLOR, scaled_pos, 5)
        else:
            # plot nodes with skyblue circle
            pygame.draw.circle(screen, Util.NODE_COLOR, scaled_pos, 10)
            font = pygame.font.SysFont(None, Util.FONT_SIZE)
            node_text = font.render(str(node['id']), True, Util.TEXT_COLOR)
            text_rect = node_text.get_rect(center=scaled_pos)
            screen.blit(node_text, text_rect)

    pygame.display.flip()

def get_node_position(node_id, nodes):
    for node in nodes:
        if node['id'] == node_id:
            return (int(node['x'] * Util.SCALE_FACTOR), int(Util.WINDOW_HEIGHT - node['y'] * Util.SCALE_FACTOR))
    return None

# sort files in ascending order
def extract_number(filename):
    match = re.search(r'\d+', filename)
    return int(match.group()) if match else 0

# read json file and plot
def play_animation():
    time.sleep(2)
    json_files = sorted(glob.glob(sys.argv[1] + "/*.json"), key=extract_number)

    for json_file in json_files:
        graph_data = load_graph_from_json(json_file) # read current json file
        draw_graph(graph_data) # plot current json file
        time.sleep(1) 

# main cycle
running = True
animation_played = False

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    if not animation_played:
        play_animation()
        animation_played = True

pygame.quit()
