static __not_inline GoLGrid *GoLUtils_alloc_std_grid (s32 left_x, s32 top_y, s32 width, s32 height)
{
	GoLGrid *gg = malloc (sizeof (GoLGrid));
	if (!gg)
	{
		fprintf (stderr, "Out of memory allocating GoLGrid object");
		return NULL;
	}
	
	Rect grid_rect;
	Rect_make (&grid_rect, left_x, top_y, width, height);
	
	if (!GoLGrid_create (gg, &grid_rect))
	{
		free (gg);
		return NULL;
	}
	
	return gg;
}

static __not_inline void GoLUtils_free_std_grid (GoLGrid **gg)
{
	if (!gg || !*gg)
		return (void) ffsc (__func__);
	
	GoLGrid_free (*gg);
	free (*gg);
	
	*gg = NULL;
}

static __not_inline int GoLUtils_or_glider (GoLGrid *gg, const Glider *glider, int consider_grid_generation)
{
	if (!gg || !glider)
		return ffsc (__func__);
	
	Glider adj_glider;
	const Glider *glider_to_use = glider;
	
	if (consider_grid_generation)
	{
		adj_glider.dir = glider->dir;
		adj_glider.lane = glider->lane;
		adj_glider.timing = glider->timing - gg->generation;
		
		glider_to_use = &adj_glider;
	}
	
	ObjCellList ocl;
	Objects_make_glider_obj_cell_list (&ocl, glider_to_use);
	
	return GoLGrid_or_obj_cell_list (gg, &ocl, 0, 0);
}

static __not_inline s32 GoLUtils_get_safe_glider_progression (const GoLGrid *target_area, s32 glider_dir, u64 *projection, s32 projection_size)
{
	if (!target_area || GoLGrid_is_empty (target_area) || glider_dir < 0 || glider_dir >= 4 || !projection)
		return ffsc (__func__);
	
	if (glider_dir == 0 || glider_dir == 2)
		GoLGrid_make_rightup_projection_noinline (target_area, projection, projection_size);
	else
		GoLGrid_make_rightdown_projection_noinline (target_area, projection, projection_size);
	
	print_bin_u64 ("word 2: ", projection [2]);
	print_bin_u64 ("word 3: ", projection [3]);
	
	if (glider_dir == 0)
		return -9 - (2 * GoLGrid_get_rightdown_pop_off (target_area, projection, projection_size));
	else if (glider_dir == 1)
		return -11 + (2 * GoLGrid_get_rightup_pop_on (target_area, projection, projection_size));
	else if (glider_dir == 2)
		return -11 + (2 * GoLGrid_get_rightdown_pop_on (target_area, projection, projection_size));
	else
		return -9 - (2 * GoLGrid_get_rightup_pop_off (target_area, projection, projection_size));
}

// Only temp_gg_1 and temp_gg_2 need to be square and have the same grid rects. Returns FALSE if clipping occurred
static __not_inline int GoLUtils_flip_diagonally_virtual (const GoLGrid *src_gg, GoLGrid *dst_gg, GoLGrid *temp_gg_1, GoLGrid *temp_gg_2)
{
	if (!src_gg || !dst_gg || !temp_gg_1 || temp_gg_1->grid_rect.height != temp_gg_1->grid_rect.width || !temp_gg_2 ||
			temp_gg_2->grid_rect.width != temp_gg_1->grid_rect.width || temp_gg_2->grid_rect.height != temp_gg_1->grid_rect.height)
		return ffsc (__func__);
	
	Rect in_grid_rect;
	GoLGrid_get_grid_rect (src_gg, &in_grid_rect);
	
	Rect in_bb;
	GoLGrid_get_bounding_box (src_gg, &in_bb);
	
	Rect temp_1_grid_rect;
	GoLGrid_get_grid_rect (temp_gg_1, &temp_1_grid_rect);
	
	int clipped = GoLGrid_copy_unmatched_noinline (src_gg, temp_gg_1, temp_1_grid_rect.left_x - in_bb.left_x, temp_1_grid_rect.top_y - in_bb.top_y);
	GoLGrid_flip_diagonally_noinline (temp_gg_1, temp_gg_2);
	clipped |= GoLGrid_copy_unmatched_noinline (temp_gg_2, dst_gg, in_bb.top_y - temp_1_grid_rect.left_x, in_bb.left_x - temp_1_grid_rect.top_y);
	
	return clipped;
}
