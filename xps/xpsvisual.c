#include "ghostxps.h"

enum { TILE_NONE, TILE_TILE, TILE_FLIP_X, TILE_FLIP_Y, TILE_FLIP_X_Y };

struct userdata
{
    xps_context_t *ctx;
    xps_resource_t *dict;
    xps_item_t *visual_tag;
};

static int
xps_paint_visual_brush(const gs_client_color *pcc, gs_state *pgs)
{
    const gs_client_pattern *ppat = gs_getpattern(pcc);
    const struct userdata *data = ppat->client_data;
    xps_context_t *ctx = data->ctx;
    xps_resource_t *dict = data->dict;
    xps_item_t *visual_tag = data->visual_tag;
    gs_state *oldpgs;

    oldpgs = ctx->pgs;
    ctx->pgs = pgs;

    xps_parse_element(ctx, dict, visual_tag);

    ctx->pgs = oldpgs;

    return 0;
}

int
xps_parse_visual_brush(xps_context_t *ctx, xps_resource_t *dict, xps_item_t *root)
{
    xps_item_t *node;
    int i, k;
    int tile_x, tile_y;

    char *opacity_att;
    char *transform_att;
    char *viewbox_att;
    char *viewport_att;
    char *tile_mode_att;
    char *viewbox_units_att;
    char *viewport_units_att;
    char *visual_att;

    xps_item_t *transform_tag = NULL;
    xps_item_t *visual_tag = NULL;

    gs_matrix transform;
    gs_rect viewbox;
    gs_rect viewport;
    float scalex, scaley;
    int tile_mode;

    opacity_att = xps_att(root, "Opacity");
    transform_att = xps_att(root, "Transform");
    viewbox_att = xps_att(root, "Viewbox");
    viewport_att = xps_att(root, "Viewport");
    tile_mode_att = xps_att(root, "TileMode");
    viewbox_units_att = xps_att(root, "ViewboxUnits");
    viewport_units_att = xps_att(root, "ViewportUnits");
    visual_att = xps_att(root, "Visual");

    for (node = xps_down(root); node; node = xps_next(node))
    {
	if (!strcmp(xps_tag(node), "VisualBrush.Transform"))
	    transform_tag = xps_down(node);
	if (!strcmp(xps_tag(node), "VisualBrush.Visual"))
	    visual_tag = xps_down(node);
    }

    xps_resolve_resource_reference(ctx, dict, &transform_att, &transform_tag);
    xps_resolve_resource_reference(ctx, dict, &visual_att, &visual_tag);

    dputs("drawing visual brush\n");

    if (!viewbox_att || !viewport_att)
	return gs_throw(-1, "missing viewbox/viewport attribute in visual brush");

    if (!visual_tag)
	return 0; /* move along, nothing to see */

    gs_make_identity(&transform);
    if (transform_att)
	xps_parse_render_transform(ctx, transform_att, &transform);
    if (transform_tag)
	xps_parse_matrix_transform(ctx, transform_tag, &transform);

    viewbox.p.x = 0.0; viewbox.p.y = 0.0;
    viewbox.q.x = 1.0; viewbox.q.y = 1.0;
    if (viewbox_att)
	xps_parse_rectangle(ctx, viewbox_att, &viewbox);

    viewport.p.x = 0.0; viewport.p.y = 0.0;
    viewport.q.x = 1.0; viewport.q.y = 1.0;
    if (viewport_att)
	xps_parse_rectangle(ctx, viewport_att, &viewport);

    tile_mode = TILE_NONE;
    if (tile_mode_att)
    {
	if (!strcmp(tile_mode_att, "None"))
	    tile_mode = TILE_NONE;
	if (!strcmp(tile_mode_att, "Tile"))
	    tile_mode = TILE_TILE;
	if (!strcmp(tile_mode_att, "FlipX"))
	    tile_mode = TILE_FLIP_X;
	if (!strcmp(tile_mode_att, "FlipY"))
	    tile_mode = TILE_FLIP_Y;
	if (!strcmp(tile_mode_att, "FlipXY"))
	    tile_mode = TILE_FLIP_X_Y;
    }

    scalex = (viewport.q.x - viewport.p.x) / (viewbox.q.x - viewbox.p.x);
    scaley = (viewport.q.y - viewport.p.y) / (viewbox.q.y - viewbox.p.y);

    gs_gsave(ctx->pgs);

    gs_concat(ctx->pgs, &transform);

    xps_begin_opacity(ctx, dict, opacity_att, NULL);

    if (tile_mode != TILE_NONE)
    {
	struct userdata foo;
	gs_client_pattern gspat;
	gs_client_color gscolor;
	gs_color_space *cs;
	gs_matrix mat;

	dprintf6("xps_parse_visual_brush [%g %g %g %g] scale=%g,%g\n",
		transform.xx, transform.xy,
		transform.yx, transform.yy,
		scalex, scaley);
	dprintf4("  viewport [%g %g %g %g]\n", viewport.p.x, viewport.p.y, viewport.q.x, viewport.q.y);
	dprintf4("  viewbox  [%g %g %g %g]\n", viewbox.p.x, viewbox.p.y, viewbox.q.x, viewbox.q.y);
	gs_currentmatrix(ctx->pgs, &mat);
	dprintf6("  current [%g %g %g %g %g %g]\n",
		mat.xx, mat.xy,
		mat.yx, mat.yy,
		mat.tx, mat.ty);

	gs_pattern1_init(&gspat);
	uid_set_UniqueID(&gspat.uid, gs_next_ids(ctx->memory, 1));
	gspat.PaintType = 1;
	gspat.TilingType = 1;
	gspat.PaintProc = xps_paint_visual_brush;
	gspat.client_data = &foo;

	foo.ctx = ctx;
	foo.dict = dict;
	foo.visual_tag = visual_tag;

	gspat.BBox.p.x = viewbox.p.x;
	gspat.BBox.p.y = viewbox.p.y;
	gspat.BBox.q.x = viewbox.q.x;
	gspat.BBox.q.y = viewbox.q.y;
	gspat.XStep = viewbox.q.x - viewbox.p.x;
	gspat.YStep = viewbox.q.y - viewbox.p.y;

	gs_translate(ctx->pgs, viewport.p.x, viewport.p.y);
	gs_scale(ctx->pgs, scalex, scaley);
	gs_translate(ctx->pgs, -viewbox.p.x, -viewbox.p.y);

	cs = gs_cspace_new_DeviceRGB(ctx->memory);
	gs_setcolorspace(ctx->pgs, cs);
	gs_make_identity(&mat);
	gs_makepattern(&gscolor, &gspat, &mat, ctx->pgs, NULL);
	gs_setpattern(ctx->pgs, &gscolor);

	// fill all area...
	// gs_setgray(ctx->pgs, 0.3); // test by filling area with gray instead of pattern
	// gs_moveto(ctx->pgs, 0, 0);
	// gs_lineto(ctx->pgs, 0, 1000);
	// gs_lineto(ctx->pgs, 1000, 1000);
	// gs_lineto(ctx->pgs, 1000, 0);
	// gs_closepath(ctx->pgs);

	xps_fill(ctx);
    }
    else
    {
	dputs("single tile\n");

	xps_clip(ctx);

	gs_translate(ctx->pgs, viewport.p.x, viewport.p.y);
	gs_scale(ctx->pgs, scalex, scaley);
	gs_translate(ctx->pgs, -viewbox.p.x, viewbox.p.y);

	gs_moveto(ctx->pgs, viewbox.p.x, viewbox.p.y);
	gs_lineto(ctx->pgs, viewbox.p.x, viewbox.q.y);
	gs_lineto(ctx->pgs, viewbox.q.x, viewbox.q.y);
	gs_lineto(ctx->pgs, viewbox.q.x, viewbox.p.y);
	gs_closepath(ctx->pgs);
	gs_clip(ctx->pgs);
	gs_newpath(ctx->pgs);

	xps_parse_element(ctx, dict, visual_tag);
    }

    xps_end_opacity(ctx, dict, opacity_att, NULL);

    gs_grestore(ctx->pgs);

    dputs("finished visual brush\n");

    return 0;
}

