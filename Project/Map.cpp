#include "Map.h"
#include "Entity.h"

Map::Map(int width, int height, std::vector<int> level_data, GLuint texture_id, float tile_size, int tile_count_x, int tile_count_y) :
m_width(width), m_height(height), m_level_data(level_data), m_texture_id(texture_id), m_tile_size(tile_size), m_tile_count_x(tile_count_x), m_tile_count_y(tile_count_y) {
    build();
}


void Map::build()
{
    // clear vectors, in case we're rebuilding
    m_vertices.clear();
    m_texture_coordinates.clear();
    
    // Since this is a 2D map, we need a nested for-loop
    for(int y_coord = 0; y_coord < m_height; y_coord++)
    {
        for(int x_coord = 0; x_coord < m_width; x_coord++)
        {   
            // Get the current tile
            int tile = m_level_data[y_coord * m_width + x_coord];
            // If the tile number is 0 i.e. not solid, skip to the next one
            if (tile == 0) continue;
            
            // Otherwise, calculate its UV-coordinates
            float u_coord = (float) (tile % m_tile_count_x) / (float) m_tile_count_x;
            float v_coord = (float) (tile / m_tile_count_x) / (float) m_tile_count_y;
            
            // And work out their dimensions and posititions
            float tile_width = 1.0f/ (float)  m_tile_count_x;
            float tile_height = 1.0f/ (float) m_tile_count_y;
            
            float x_offset = -(m_tile_size / 2); // From center of tile
            float y_offset =  (m_tile_size / 2); // From center of tile
            
            // So we can store them inside our std::vectors
            m_vertices.insert(m_vertices.end(), {
                x_offset + (m_tile_size * x_coord),  y_offset +  -m_tile_size * y_coord,
                x_offset + (m_tile_size * x_coord),  y_offset + (-m_tile_size * y_coord) - m_tile_size,
                x_offset + (m_tile_size * x_coord) + m_tile_size, y_offset + (-m_tile_size * y_coord) - m_tile_size,
                x_offset + (m_tile_size * x_coord), y_offset + -m_tile_size * y_coord,
                x_offset + (m_tile_size * x_coord) + m_tile_size, y_offset + (-m_tile_size * y_coord) - m_tile_size,
                x_offset + (m_tile_size * x_coord) + m_tile_size, y_offset +  -m_tile_size * y_coord
            });
            
            m_texture_coordinates.insert(m_texture_coordinates.end(), {
                u_coord, v_coord,
                u_coord, v_coord + (tile_height),
                u_coord + tile_width, v_coord + (tile_height),
                u_coord, v_coord,
                u_coord + tile_width, v_coord + (tile_height),
                u_coord + tile_width, v_coord
            });
        }
    }
    
    // The bounds are dependent on the size of the tiles
    m_left_bound   = 0 - (m_tile_size / 2);
    m_right_bound  = (m_tile_size * m_width) - (m_tile_size / 2);
    m_top_bound    = 0 + (m_tile_size / 2);
    m_bottom_bound = -(m_tile_size * m_height) + (m_tile_size / 2);
}


void Map::update(Entity* player, float delta_time) {
    
    if (player->is_digging()) {
        m_dig_count += delta_time;
        glm::vec3 player_pos = player->get_position();

        int tile_x = floor((player_pos.x + (m_tile_size / 2))  / m_tile_size);
        int tile_y = -(ceil(player_pos.y - (m_tile_size / 2))) / m_tile_size;

        if (player->is_digging_left()) {
            tile_x -= 1;
        }
        else if (player->is_digging_right()) {
            tile_x += 1;
        }
        else {
            tile_y += 1;
        }
        if (m_dig_count >= m_tile_hp) {
            delete_tile(player, tile_x, tile_y);
            m_dig_count = 0;
        }
    }
}

void Map::delete_tile(Entity* player, int tile_x, int tile_y) {
    
//    std::cout << "width, heigth: " << m_width << ", " << m_height << std::endl;
//    std::cout << "tile: " << tile_x << ", " << tile_y << std::endl;
    
    if ((player->is_digging_right() && tile_x < m_width) || (player->is_digging_left() && tile_x > 0)){
        m_level_data[tile_y * m_width + tile_x] = 0;
        build();
    }
    else if (!player->is_digging_right() && !player->is_digging_left() && tile_y < m_height - 1) {
        m_level_data[tile_y * m_width + tile_x] = 0;
        build();
    }
}

void Map::render(ShaderProgram *program)
{
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);
    
    glUseProgram(program->get_program_id());
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, m_vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, m_texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    
    glDrawArrays(GL_TRIANGLES, 0, (int) m_vertices.size() / 2);
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool Map::is_solid(glm::vec3 position, float *penetration_x, float *penetration_y)
{
    // The penetration between the map and the object
    // The reason why these are pointers is because we want to reassign values
    // to them in case that we are colliding. That way the object that originally
    // passed them as values will keep track of these distances
    // inb4: we're passing by reference
    *penetration_x = 0;
    *penetration_y = 0;
    
    // If we are out of bounds, it is not solid
    if (position.x < m_left_bound || position.x > m_right_bound)  return false;
    if (position.y > m_top_bound  || position.y < m_bottom_bound) return false;
    
    int tile_x = floor((position.x + (m_tile_size / 2))  / m_tile_size);
    int tile_y = -(ceil(position.y - (m_tile_size / 2))) / m_tile_size; // Our array counts up as Y goes down.
    
    // If the tile index is negative or greater than the dimensions, it is not solid
    if (tile_x < 0 || tile_x >= m_width)  return false;
    if (tile_y < 0 || tile_y >= m_height) return false;
    
    // If the tile index is 0 i.e. an open space, it is not solid
    int tile = m_level_data[tile_y * m_width + tile_x];
    if (tile == 0) return false;
    
    // And we likely have some overlap
    float tile_center_x = (tile_x  * m_tile_size);
    float tile_center_y = -(tile_y * m_tile_size);
    
    // And because we likely have some overlap, we adjust for that
    *penetration_x = (m_tile_size / 2) - fabs(position.x - tile_center_x);
    *penetration_y = (m_tile_size / 2) - fabs(position.y - tile_center_y);
    
    return true;
}

bool Map::is_triangle(glm::vec3 position) {
    
    int tile_x = floor((position.x + (m_tile_size / 2))  / m_tile_size);
    int tile_y = -(ceil(position.y - (m_tile_size / 2))) / m_tile_size; // Our array counts up as Y goes down.
    
    // If the tile index is 0 i.e. an open space, it is not solid
    int tile = m_level_data[tile_y * m_width + tile_x];
    if (tile == 1 || tile == 2 || tile == 8 || tile == 9 || tile == 15 || tile == 16) {
        return true;
    }
    return false;
}

