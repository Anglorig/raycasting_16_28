void draw_scene()
    {
        fmt = SDL_AllocFormat(format);
        SDL_LockTexture(buffer, NULL, &pixels, &pitch);
        for(int x = 0; x < (SCREEN_WIDTH); x++)
        {
            camera_X = (2 * x) / double(SCREEN_WIDTH) - 1;
            ray_pos_X = player_pos_X;
            ray_pos_Y = player_pos_Y;
            ray_dir_X = player_dir_X + player_cam_X * camera_X;
            ray_dir_Y = player_dir_Y + player_cam_Y * camera_X;
            map_X = int(ray_pos_X);
            map_Y = int(ray_pos_Y);

            delta_DistX = sqrt(1 + (ray_dir_Y * ray_dir_Y) / (ray_dir_X * ray_dir_X));
            delta_DistY = sqrt(1 + (ray_dir_X * ray_dir_X) / (ray_dir_Y * ray_dir_Y));

            hit = 0;
            if(ray_dir_X < 0)
            {
                step_X = -1;
                side_DistX = (ray_pos_X - map_X) * delta_DistX;
            }
            else
            {
                step_X = 1;
                side_DistX = (map_X + 1.0 - ray_pos_X) * delta_DistX;
            }
            if(ray_dir_Y < 0)
            {
                step_Y = -1;
                side_DistY = (ray_pos_Y - map_Y) * delta_DistY;
            }
            else
            {
                step_Y = 1;
                side_DistY = (map_Y + 1.0 - ray_pos_Y) * delta_DistY;
            }

            while(hit == 0)
            {
                if(side_DistX < side_DistY)
                {
                    side_DistX += delta_DistX;
                    map_X += step_X;
                    side = 0;
                }
                else
                {
                    side_DistY += delta_DistY;
                    map_Y += step_Y;
                    side = 1;
                }
                if(MAP[map_X][map_Y] > 0)
                {
                    hit = 1;
                }
            }
            if(side == 0) perp_WallDist = (map_X - ray_pos_X + (1 - step_X) / 2) / ray_dir_X;
            else          perp_WallDist = (map_Y - ray_pos_Y + (1 - step_Y) / 2) / ray_dir_Y;

            line_height = (int)(SCREEN_HEIGHT / perp_WallDist);
            draw_start = (-line_height / 2) + (SCREEN_HEIGHT / 2);
            if(draw_start < 0) draw_start = 0;
            draw_end   = (line_height / 2) + (SCREEN_HEIGHT / 2);
            if(draw_end >= SCREEN_HEIGHT)draw_end = SCREEN_HEIGHT - 1;

            /*WHAT TO RENDER?*/
            int tex_selector;
            switch(MAP[map_X][map_Y])
            {
            case 1:
                tex_selector = 0;
                break;
            case 2:
                tex_selector = 64;
                break;
            case 3:
                tex_selector = 128;
                break;
            case 4:
                tex_selector = 192;
                break;
            case 5:
                tex_selector = 256;
                break;
            case 6:
                tex_selector = 320;
                break;
            case 7:
                tex_selector = 384;
                break;
            default:
                tex_selector = 0;
            }
            /*WHERE WAS THE WALL HIT?*/
            double wallX;
            if(side == 0) wallX = ray_pos_Y + perp_WallDist * ray_dir_Y;
            else          wallX = ray_pos_X + perp_WallDist * ray_dir_X;

            wallX -= floor(wallX);

            int texX = int(wallX * double(texWidth));
            if(side == 0 && ray_dir_X > 0) texX = texWidth - texX - 1;
            else if(side == 1 && ray_dir_Y < 0) texX = texWidth - texX - 1;

            for(int y = 0; y < SCREEN_HEIGHT; y++)
            {
                    int d = y * 256 - SCREEN_HEIGHT * 128 + line_height * 128;
                    int texY = ((d * texHeight) / line_height) / 256;

                    unsigned int texel_pos = texX + ((texY + tex_selector) * texWidth);
                    if(texel_pos > texel_limit) texel_pos = texel_limit;

                    if(y > draw_start && y < draw_end)
                    {
                        if(side == 1)((Uint32*)pixels)[x + (y * SCREEN_WIDTH)] = (texels[texel_pos] >> 1) & 8355711;
                        else ((Uint32*)pixels)[x + (y * SCREEN_WIDTH)] = texels[texel_pos];

                    }
                    //else ((Uint32*)pixels)[(Uint32)x + (y * SCREEN_WIDTH)] = SDL_MapRGB(fmt, 255, 25, 25);
            }
            ZBuffer[x] = perp_WallDist;

            /*FLOOR & CEILING CASTING*/
            double floor_X_wall, floor_Y_wall;
            if(side == 0)
            {
                if(ray_dir_X > 0) floor_X_wall = map_X;
                else              floor_X_wall = map_X + 1.0;
                floor_Y_wall = map_Y + wallX;
            }
            else
            {
                if(ray_dir_Y > 0) floor_Y_wall = map_Y;
                else              floor_Y_wall = map_Y + 1.0;
                floor_X_wall = map_X + wallX;
            }

            double distWall, distPlayer, currentDist;

            distWall = perp_WallDist;
            distPlayer = 0.0;

            if(draw_end < 0) draw_end = SCREEN_HEIGHT;

            for(int y = draw_end; y < SCREEN_HEIGHT; y++)
            {
                currentDist = SCREEN_HEIGHT / (2.0 * y - SCREEN_HEIGHT);
                double weight = (currentDist - distPlayer) / (distWall - distPlayer);
                double currentFloorX = weight * floor_X_wall + (1.0 - weight) * player_pos_X;
                double currentFloorY = weight * floor_Y_wall + (1.0 - weight) * player_pos_Y;

                int floorTexX = int(currentFloorX * texWidth) % texWidth;
                int floorTexY = int(currentFloorY * texHeight) % texHeight;

                unsigned int texel_pos = floorTexX + ((floorTexY + 128) * texWidth);
                ((Uint32*)pixels)[x + (y * SCREEN_WIDTH)] = texels[texel_pos];
                texel_pos = floorTexX + ((floorTexY + 64) * texWidth);
                ((Uint32*)pixels)[x + ((SCREEN_HEIGHT - y) * SCREEN_WIDTH)] = texels[texel_pos];
            }
        }

        draw_sprites();
        SDL_UnlockTexture(buffer);
        SDL_FreeFormat(fmt);
        SDL_RenderCopy(gRenderer, buffer, NULL, NULL);
    }