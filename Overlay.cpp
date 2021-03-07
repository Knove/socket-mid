#include "overlay.h"
#include <d3dtypes.h>
#include <stdio.h>
#include <vector>

LPDIRECT3DDEVICE9 d3ddev1;
LPDIRECT3DVERTEXBUFFER9 g_pVB1;

LPD3DXFONT pFont;
LPD3DXFONT pFont2;
ID3DXLine* d3l;

void Init(LPDIRECT3DDEVICE9 d, LPDIRECT3DVERTEXBUFFER9 pvb, ID3DXLine* ID3DXLine) {
	d3ddev1 = d;
	g_pVB1 = pvb;
	d3l = ID3DXLine;
	D3DXCreateFontA(d3ddev1, 12, 0, FW_MEDIUM, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Purista-light", &pFont);
	D3DXCreateFontA(d3ddev1, 15, 0, FW_MEDIUM, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "icomoon", &pFont2);
}

void String(int x, int y, D3DCOLOR colour, bool outlined, const char* string, ...)
{
	va_list args;
	char cBuffer[256];

	va_start(args, string);
	vsprintf_s(cBuffer, string, args);
	va_end(args);

	RECT pRect;
	if (outlined) {
		pRect.left = x - 1;
		pRect.top = y;
		pFont->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
		pRect.left = x + 1;
		pRect.top = y;
		pFont->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
		pRect.left = x;
		pRect.top = y - 1;
		pFont->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
		pRect.left = x;
		pRect.top = y + 1;
		pFont->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
	}
	pRect.left = x;
	pRect.top = y;
	pFont->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, colour);
}

void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color)
{
	std::vector<SD3DVertex> circle(resolution + 2);

	float angle = rotate * D3DX_PI / 180;
	float pi;

	if (type == 1) pi = D3DX_PI;        // Full circle
	if (type == 2) pi = D3DX_PI / 2;      // 1/2 circle
	if (type == 3) pi = D3DX_PI / 4;   // 1/4 circle

	for (int i = 0; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(i * (2 * pi / resolution)));
		circle[i].y = (float)(y - radius * sin(i * (2 * pi / resolution)));
		circle[i].z = 0;
		circle[i].rhw = 1;
		circle[i].colour = color;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		circle[i].x = x + cos(angle) * (circle[i].x - x) - sin(angle) * (circle[i].y - y);
		circle[i].y = y + sin(angle) * (circle[i].x - x) + cos(angle) * (circle[i].y - y);
	}

	d3ddev1->CreateVertexBuffer((resolution + 2) * sizeof(SD3DVertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB1, NULL);

	VOID* pVertices;
	g_pVB1->Lock(0, (resolution + 2) * sizeof(SD3DVertex), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(SD3DVertex));
	g_pVB1->Unlock();


	d3ddev1->SetTexture(0, NULL);
	d3ddev1->SetPixelShader(NULL);
	if (smoothing)
	{
		d3ddev1->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		d3ddev1->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	}
	d3ddev1->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	d3ddev1->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3ddev1->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	d3ddev1->SetStreamSource(0, g_pVB1, 0, sizeof(SD3DVertex));
	d3ddev1->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	d3ddev1->DrawPrimitive(D3DPT_LINESTRIP, 0, resolution);
	if (g_pVB1 != NULL) g_pVB1->Release();
}


void draw_box(float x, float y, float width, float height, D3DCOLOR color)
{
	D3DXVECTOR2 points[5];
	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x + width, y);
	points[2] = D3DXVECTOR2(x + width, y + height);
	points[3] = D3DXVECTOR2(x, y + height);
	points[4] = D3DXVECTOR2(x, y);
	d3l->SetWidth(1);
	d3l->Draw(points, 5, color);
}