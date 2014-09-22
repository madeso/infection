#include "genesis.h"

/*
	This file is provided as a debugging aid.  It contains code for drawing an axial
	aligned bounding box.  You can use this code as you see fit.
*/

static	void	DrawFace(geWorld *World, const geVec3d **Verts)
{
	GE_LVertex	LVerts[4];
	int			i;

	for	(i = 0; i < 4; i++)
	{
		LVerts[i].r = 40.0f;
		LVerts[i].g = 40.0f;
		LVerts[i].b = 80.0f + 20.0f * (geFloat)i;
		LVerts[i].a = 128.0f;
		LVerts[i].X = Verts[i]->X;
		LVerts[i].Y = Verts[i]->Y;
		LVerts[i].Z = Verts[i]->Z;
	}

	geWorld_AddPolyOnce(World, &LVerts[0], 4, NULL, GE_GOURAUD_POLY, /*GE_FX_TRANSPARENT*/ 0, 1.0f);
}

void	DrawBoundBox_8Faces(geWorld *World, const geVec3d *Pos, const geVec3d *Min, const geVec3d *Max)
{
	geFloat	dx;
	geFloat	dy;
	geFloat	dz;
static	geVec3d		Verts[8];
static	geVec3d *	Faces[6][4] =
{
	{ &Verts[0], &Verts[1], &Verts[2], &Verts[3] },	//Top
	{ &Verts[4], &Verts[5], &Verts[6], &Verts[7] },	//Bottom
	{ &Verts[3], &Verts[2], &Verts[6], &Verts[7] }, //Side
	{ &Verts[1], &Verts[0], &Verts[4], &Verts[5] }, //Side
	{ &Verts[0], &Verts[3], &Verts[7], &Verts[4] }, //Front
	{ &Verts[2], &Verts[1], &Verts[5], &Verts[6] }, //Back
};
	int			i;

	for	(i = 0; i < 8; i++)
		geVec3d_Add(Pos, Min, &Verts[i]);

	dx = Max->X - Min->X;
	dy = Max->Y - Min->Y;
	dz = Max->Z - Min->Z;

	Verts[0].Y += dy;
	Verts[3].Y += dy;
	Verts[3].X += dx;
	Verts[7].X += dx;

	Verts[1].Y += dy;
	Verts[1].Z += dz;
	Verts[5].Z += dz;
	Verts[6].Z += dz;
	Verts[6].X += dx;

	Verts[2].X += dx;
	Verts[2].Y += dy;
	Verts[2].Z += dz;

	for	(i = 0; i < 6; i++)
		DrawFace(World, &Faces[i][0]);
}

#define FACE(a, b, c, d) { &Verts[a], &Verts[b], &Verts[c], &Verts[d] },
#define LINE_WIDTH 1.0f;
void	DrawBoundBox(geWorld *World, const geVec3d *Pos, const geVec3d *Min, const geVec3d *Max)
{
	geFloat	dx;
	geFloat	dy;
	geFloat	dz;

	geFloat x1, y1, z1;
	geFloat x2, y2, z2;
	geFloat x3, y3, z3;
	geFloat x4, y4, z4;

static	geVec3d		Verts[40];
static	geVec3d *	Faces[48][4] =
{
	FACE(2, 0, 8, 11 )
	FACE(0, 4, 9, 8 )
	FACE(4, 6, 10, 9 )
	FACE(6, 2, 11, 10 )
	FACE(5, 1, 12, 15 )
	FACE(1, 3, 13, 12 )
	FACE(3, 7, 14, 13 )
	FACE(7, 5, 15, 14 )
	FACE(1, 0, 16, 19 )
	FACE(0, 2, 17, 16 )
	FACE(2, 3, 18, 17 )
	FACE(3, 1, 19, 18 )
	FACE(6, 4, 20, 23 )
	FACE(4, 5, 21, 20 )
	FACE(5, 7, 22, 21 )
	FACE(7, 6, 23, 22 )
	FACE(3, 2, 24, 27 )
	FACE(2, 6, 25, 24 )
	FACE(6, 7, 26, 25 )
	FACE(7, 3, 27, 26 )
	FACE(4, 0, 28, 31 )
	FACE(0, 1, 29, 28 )
	FACE(1, 5, 30, 29 )
	FACE(5, 4, 31, 30 )
	FACE(11, 8, 32, 35 )
	FACE(8, 9, 33, 32 )
	FACE(9, 10, 34, 33 )
	FACE(10, 11, 35, 34 )
	FACE(15, 12, 36, 39 )
	FACE(12, 13, 37, 36 )
	FACE(13, 14, 38, 37 )
	FACE(14, 15, 39, 38 )
	FACE(19, 16, 32, 36 )
	FACE(16, 17, 35, 32 )
	FACE(17, 18, 37, 35 )
	FACE(18, 19, 36, 37 )
	FACE(23, 20, 33, 34 )
	FACE(20, 21, 39, 33 )
	FACE(21, 22, 38, 39 )
	FACE(22, 23, 34, 38 )
	FACE(27, 24, 35, 37 )
	FACE(24, 25, 34, 35 )
	FACE(25, 26, 38, 34 )
	FACE(26, 27, 37, 38 )
	FACE(31, 28, 32, 33 )
	FACE(28, 29, 36, 32 )
	FACE(29, 30, 39, 36 )
	FACE(30, 31, 33, 39 )
};
	int			i;

	for	(i = 0; i < 40; i++)
		geVec3d_Add(Pos, Min, &Verts[i]);

	dx = Max->X - Min->X;
	dy = Max->Y - Min->Y;
	dz = Max->Z - Min->Z;

	x1 = 0.0f;
	y1 = 0.0f;
	z1 = 0.0f;

	x2 = LINE_WIDTH;
	y2 = LINE_WIDTH;
	z2 = LINE_WIDTH;

	x3 = dx - LINE_WIDTH;
	y3 = dy - LINE_WIDTH;
	z3 = dz - LINE_WIDTH;

	x4 = dx;
	y4 = dy;
	z4 = dz;

	// Face #1
	Verts[0].X += x1;
	Verts[0].Y += y1;
	Verts[0].Z += z1;
	
	// Face #2
	Verts[1].X += x1;
	Verts[1].Y += y1;
	Verts[1].Z += z4;
	
	// Face #3
	Verts[2].X += x1;
	Verts[2].Y += y4;
	Verts[2].Z += z1;
	
	// Face #4
	Verts[3].X += x1;
	Verts[3].Y += y4;
	Verts[3].Z += z4;
	
	// Face #5
	Verts[4].X += x4;
	Verts[4].Y += y1;
	Verts[4].Z += z1;
	
	// Face #6
	Verts[5].X += x4;
	Verts[5].Y += y1;
	Verts[5].Z += z4;
	
	// Face #7
	Verts[6].X += x4;
	Verts[6].Y += y4;
	Verts[6].Z += z1;
	
	// Face #8
	Verts[7].X += x4;
	Verts[7].Y += y4;
	Verts[7].Z += z4;
	
	// Face #9
	Verts[8].X += x2;
	Verts[8].Y += y2;
	Verts[8].Z += z1;
	
	// Face #10
	Verts[9].X += x3;
	Verts[9].Y += y2;
	Verts[9].Z += z1;
	
	// Face #11
	Verts[10].X += x3;
	Verts[10].Y += y3;
	Verts[10].Z += z1;
	
	// Face #12
	Verts[11].X += x2;
	Verts[11].Y += y3;
	Verts[11].Z += z1;
	
	// Face #13
	Verts[12].X += x2;
	Verts[12].Y += y2;
	Verts[12].Z += z4;
	
	// Face #14
	Verts[13].X += x2;
	Verts[13].Y += y3;
	Verts[13].Z += z4;
	
	// Face #15
	Verts[14].X += x3;
	Verts[14].Y += y3;
	Verts[14].Z += z4;
	
	// Face #16
	Verts[15].X += x3;
	Verts[15].Y += y2;
	Verts[15].Z += z4;
	
	// Face #17
	Verts[16].X += x1;
	Verts[16].Y += y2;
	Verts[16].Z += z2;
	
	// Face #18
	Verts[17].X += x1;
	Verts[17].Y += y3;
	Verts[17].Z += z2;
	
	// Face #19
	Verts[18].X += x1;
	Verts[18].Y += y3;
	Verts[18].Z += z3;
	
	// Face #20
	Verts[19].X += x1;
	Verts[19].Y += y2;
	Verts[19].Z += z3;
	
	// Face #21
	Verts[20].X += x4;
	Verts[20].Y += y2;
	Verts[20].Z += z2;
	
	// Face #22
	Verts[21].X += x4;
	Verts[21].Y += y2;
	Verts[21].Z += z3;
	
	// Face #23
	Verts[22].X += x4;
	Verts[22].Y += y3;
	Verts[22].Z += z3;
	
	// Face #24
	Verts[23].X += x4;
	Verts[23].Y += y3;
	Verts[23].Z += z2;
	
	// Face #25
	Verts[24].X += x2;
	Verts[24].Y += y4;
	Verts[24].Z += z2;
	
	// Face #26
	Verts[25].X += x3;
	Verts[25].Y += y4;
	Verts[25].Z += z2;
	
	// Face #27
	Verts[26].X += x3;
	Verts[26].Y += y4;
	Verts[26].Z += z3;
	
	// Face #28
	Verts[27].X += x2;
	Verts[27].Y += y4;
	Verts[27].Z += z3;
	
	// Face #29
	Verts[28].X += x2;
	Verts[28].Y += y1;
	Verts[28].Z += z2;
	
	// Face #30
	Verts[29].X += x2;
	Verts[29].Y += y1;
	Verts[29].Z += z3;
	
	// Face #31
	Verts[30].X += x3;
	Verts[30].Y += y1;
	Verts[30].Z += z3;
	
	// Face #32
	Verts[31].X += x3;
	Verts[31].Y += y1;
	Verts[31].Z += z2;
	
	// Face #33
	Verts[32].X += x2;
	Verts[32].Y += y2;
	Verts[32].Z += z2;
	
	// Face #34
	Verts[33].X += x3;
	Verts[33].Y += y2;
	Verts[33].Z += z2;
	
	// Face #35
	Verts[34].X += x3;
	Verts[34].Y += y3;
	Verts[34].Z += z2;
	
	// Face #36
	Verts[35].X += x2;
	Verts[35].Y += y3;
	Verts[35].Z += z2;
	
	// Face #37
	Verts[36].X += x2;
	Verts[36].Y += y2;
	Verts[36].Z += z3;
	
	// Face #38
	Verts[37].X += x2;
	Verts[37].Y += y3;
	Verts[37].Z += z3;
	
	// Face #39
	Verts[38].X += x3;
	Verts[38].Y += y3;
	Verts[38].Z += z3;
	
	// Face #40
	Verts[39].X += x3;
	Verts[39].Y += y2;
	Verts[39].Z += z3;
	


	for	(i = 0; i < 40; i++)
		DrawFace(World, &Faces[i][0]);
}
#undef FACE
#undef LINE_WIDTH