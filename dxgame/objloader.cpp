// from http://www.opengl-tutorial.org/

#include "stdafx.h"

#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>

#include <directxmath.h>

#include "objloader.hpp"
#include "vertex.h"

using namespace std;
using namespace DirectX;

bool loadOBJ(const wchar_t * path, 
    vector<Vertex> & out_vertices)
{
    wcout << L"Loading OBJ file " <<  path << endl;

    vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    vector<DirectX::XMFLOAT3> temp_vertices; 
    vector<DirectX::XMFLOAT2> temp_uvs;
    vector<DirectX::XMFLOAT3> temp_normals;

    FILE *file = nullptr;
    errno_t rc = _wfopen_s(&file, path, L"r");
    if( rc || file == NULL ){
        wcout << L"Could not open file: " << path << endl;
        return false;
    }

    int line = 0;
    while( 1 ){

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf_s(file, "%s", lineHeader, 128);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // ignore warnings for fscanf() when reading numbers:
#pragma warning(disable : 4996)

        // else : parse lineHeader
        
        if ( strcmp( lineHeader, "v" ) == 0 ){
            DirectX::XMFLOAT3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            DirectX::XMFLOAT2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            temp_uvs.push_back(uv);
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            DirectX::XMFLOAT3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            //string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int pos = ftell(file);
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches == 9)
            {
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices    .push_back(uvIndex[0]);
                uvIndices    .push_back(uvIndex[1]);
                uvIndices    .push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            } else {
                fseek(file, pos, SEEK_SET);
                int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                if (matches == 3)
                {
                    vertexIndices.push_back(vertexIndex[0]);
                    vertexIndices.push_back(vertexIndex[1]);
                    vertexIndices.push_back(vertexIndex[2]);
                } else
                {
                    printf("Line %d; File can't be read by our simple parser :-( Try exporting with other options\n", line);
                    return false;
                }
            }
        }else{
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }

        ++line;
    }

    out_vertices.reserve(vertexIndices.size()); // allocate all the memory needed a head of time to make C++ less sad

    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){
        DirectX::XMFLOAT2 uv;
        DirectX::XMFLOAT3 normal;

        unsigned int vertexIndex = vertexIndices[i];
        DirectX::XMFLOAT3 vertex = temp_vertices[ vertexIndex-1 ];

        if (i < uvIndices.size())
        {
            unsigned int uvIndex = uvIndices[i];
            uv = temp_uvs[ uvIndex-1 ];
        } else
        {
            uv.x = uv.y = 0;
        }

        if (i < normalIndices.size())
        {
            unsigned int normalIndex = normalIndices[i];
            normal = temp_normals[ normalIndex-1 ];
        } else
        {
            normal.x = normal.y = 0;
            normal.z = 1;
        }
        

        // pack into DirectX-friendly Vertex structure
        Vertex v;
        v.pos = vertex; 
        v.tex0 = uv;
        v.tex1.x = v.tex1.y = 0;
        v.normal = normal;
        
        // Put the attributes in buffer
        out_vertices.push_back(v);
    }

    return true;
}
