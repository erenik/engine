// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"

GMBufferMesh::GMBufferMesh(Mesh * iMesh) : GraphicsMessage(GM_BUFFER_MESH) {
	mesh = iMesh;
}

void GMBufferMesh::Process(){
	mesh->Bufferize();
}