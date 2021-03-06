#ifndef __RENDER3D_COLLADA_PARSER_H__
#define __RENDER3D_COLLADA_PARSER_H__

#include <libxml/tree.h>
#include <vector>
#include "render3d/renderengine.h"

namespace render3d {

	class ColladaParser {
		struct ParsingModel {
			std::vector<float> vertices;
			std::vector<float> normals;
			std::vector<float> texCoords;
			std::vector<float> colors;
			std::vector<int> dataIndex;

			//offsets in dataIndex
			int vertexOffset;
			int normalOffset;
			int texCoordOffset;
			int colorOffset;
			int inputCount;
		};

		RenderEngine &m_engine;
		std::vector<ParsingModel*> m_models;

		xmlNodePtr getFirstChildNode(xmlNodePtr currentNode);
		xmlNodePtr getFirstChildNode(xmlNodePtr currentNode, const char *nodeName);
		xmlNodePtr getNextNode(xmlNodePtr currentNode);
		xmlNodePtr getNextNode(xmlNodePtr currentNode, const char *nodeName);

		void parseIntArray(xmlNodePtr curNode, std::vector<int> &out);
		void parseFloatArray(xmlNodePtr curNode, std::vector<float> &out);

		void readModel(xmlNodePtr rootNode);
		void readVertices(xmlNodePtr meshNode, ParsingModel* model);
		void readNormals(xmlNodePtr meshNode, const char *id, ParsingModel* model);
		void readTextureCoords(xmlNodePtr meshNode, const char *id, ParsingModel* model);
		void readColors(xmlNodePtr meshNode, const char *id, ParsingModel* model);

		int assembleModel(ParsingModel *model);

		public:
			ColladaParser(RenderEngine &engine);
			std::vector<int> parse(const char *fileName);
	};

}

#endif