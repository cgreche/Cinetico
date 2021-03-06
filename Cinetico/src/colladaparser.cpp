
#include "colladaparser.h"

#include <string>
#include <sstream>
#include <cstring>

#define XMLCHAR(x) ((const xmlChar*)x)
#define XMLCHAR2CHAR(x) ((const char*)x)
#define XMLCHAR_SAFEFREE(x) if(x) xmlFree(x)

namespace render3d {
	
	xmlNodePtr ColladaParser::getFirstChildNode(xmlNodePtr currentNode)
	{
		if (currentNode && currentNode->type == XML_ELEMENT_NODE) {
			xmlNodePtr node = currentNode->children;
			if (node && node->type != XML_ELEMENT_NODE)
				node = getNextNode(node);
			return node;
		}

		return NULL;
	}

	xmlNodePtr ColladaParser::getNextNode(xmlNodePtr currentNode)
	{
		if (currentNode) {
			xmlNodePtr node = currentNode->next;
			while (node != NULL) {
				if (node->type == XML_ELEMENT_NODE)
					return node;

				node = node->next;
			}
		}
		return NULL;
	}

	xmlNodePtr ColladaParser::getFirstChildNode(xmlNodePtr currentNode, const char *nodeName)
	{
		if (currentNode) {
			xmlNodePtr node = getFirstChildNode(currentNode);
			if (node && nodeName) {
				do {
					if (strcmp(XMLCHAR2CHAR(node->name), nodeName) == 0)
						return node;
					node = getNextNode(node);
				} while (node);
			}
			return node;
		}

		return NULL;
	}

	xmlNodePtr ColladaParser::getNextNode(xmlNodePtr currentNode, const char *nodeName)
	{
		if (currentNode) {
			xmlNodePtr node = getNextNode(currentNode);
			if (node) {
				do {
					if (strcmp(XMLCHAR2CHAR(node->name), nodeName) == 0)
						return node;
					node = getNextNode(node);
				} while (node);
			}
		}
		return NULL;
	}


	void ColladaParser::parseIntArray(xmlNodePtr curNode, std::vector<int> &out) {
		xmlChar *content = curNode->content;

		std::string str = XMLCHAR2CHAR(content);
		int buf; // Have a buffer string
		std::stringstream ss(str); // Insert the string into a stream

		while (ss >> buf)
			out.push_back(buf);
	}

	void ColladaParser::parseFloatArray(xmlNodePtr curNode, std::vector<float> &out) {
		xmlChar *content = curNode->content;

		std::string str = XMLCHAR2CHAR(content);
		float buf;
		std::stringstream ss(str); // Insert the string into a stream

		while (ss >> buf) {
			out.push_back(buf);
		}
	}

	void ColladaParser::readVertices(xmlNodePtr meshNode, ParsingModel* model) {
		if (!meshNode)
			return;

		xmlNodePtr child = getFirstChildNode(meshNode, "vertices");
		if (child) {
			child = getFirstChildNode(child, "input");
			if (child) {
				xmlChar *propSource = xmlGetProp(child, XMLCHAR("source"));
				if (propSource) {
					child = getFirstChildNode(meshNode, "source");
					while (child) {
						xmlAttrPtr propId = xmlHasProp(child, XMLCHAR("id"));
						if(propId && propId->children && xmlStrcmp(propId->children->content, &propSource[1])==0) {
							child = getFirstChildNode(child, "float_array");
							if (child) {
								child = child->children;
								if (child->type == XML_TEXT_NODE) {
									parseFloatArray(child, model->vertices);
								}
							}
							break;
						}
						child = getNextNode(child, "source");
					}
					xmlFree(propSource);
				}
			}
		}
	}

	void ColladaParser::readNormals(xmlNodePtr meshNode, const char *id, ParsingModel* model) {
		if (!meshNode)
			return;

		xmlNodePtr child = getFirstChildNode(meshNode, "source");
		while (child) {
			xmlAttrPtr attrId = xmlHasProp(child, XMLCHAR("id"));
			if (attrId && attrId->children && xmlStrcmp(attrId->children->content, XMLCHAR(id)) == 0) {
				child = getFirstChildNode(child, "float_array");
				if (child) {
					child = child->children;
					if (child->type == XML_TEXT_NODE) {
						parseFloatArray(child, model->normals);
						return;
					}
				}
			}

			child = getNextNode(child, "source");
		}
	}

	void ColladaParser::readTextureCoords(xmlNodePtr meshNode, const char *id, ParsingModel* model) {
		if (!meshNode)
			return;

		xmlNodePtr child = getFirstChildNode(meshNode, "source");
		while (child) {
			xmlAttrPtr attrId = xmlHasProp(child, XMLCHAR("id"));
			if (attrId && attrId->children && xmlStrcmp(attrId->children->content, XMLCHAR(id)) == 0) {
				child = getFirstChildNode(child, "float_array");
				if (child) {
					child = child->children;
					if (child->type == XML_TEXT_NODE) {
						parseFloatArray(child, model->texCoords);
						return;
					}
				}
			}

			child = getNextNode(child, "source");
		}
	}

	void ColladaParser::readColors(xmlNodePtr meshNode, const char *id, ParsingModel* model) {
		if (!meshNode)
			return;

		xmlNodePtr child = getFirstChildNode(meshNode, "source");
		while (child) {
			xmlAttrPtr attrId = xmlHasProp(child, XMLCHAR("id"));
			if(attrId && attrId->children && xmlStrcmp(attrId->children->content, XMLCHAR(id))==0) {
				child = getFirstChildNode(child, "float_array");
				if (child) {
					child = child->children;
					if (child->type == XML_TEXT_NODE) {
						parseFloatArray(child, model->colors);
						return;
					}
				}
			}

			child = getNextNode(child, "source");
		}
	}

	void ColladaParser::readModel(xmlNodePtr rootNode) {
		xmlNodePtr node;

		std::string normalId;
		std::string texCoordId;
		std::string colorId;


		node = getFirstChildNode(rootNode, "library_geometries");
		if (node) {
			node = getFirstChildNode(node, "geometry");
			if (node) {
				do {
					ParsingModel *model = new ParsingModel();
					int vertexOffset = 0;
					int normalOffset = -1;
					int texCoordOffset = -1;
					int colorOffset = -1;
					int inputCount = 1; //VERTEX already counted
					xmlNodePtr meshNode = getFirstChildNode(node, "mesh");
					if (meshNode) {
						xmlNodePtr triangleChild = getFirstChildNode(meshNode, "triangles");
						if (triangleChild) {
							xmlNodePtr child = getFirstChildNode(triangleChild, "input");
							while (child) {
								xmlChar *prop = xmlGetProp(child, XMLCHAR("semantic"));
								if (prop) {
									xmlChar *propSource = xmlGetProp(child, XMLCHAR("source"));
									xmlChar *propOffset = xmlGetProp(child, XMLCHAR("offset"));
									if (propSource && propOffset) {
										int offs;
										sscanf(XMLCHAR2CHAR(propOffset), "%d", &offs);
										if (strcmp(XMLCHAR2CHAR(prop), "NORMAL") == 0) {
											normalId = XMLCHAR2CHAR(&propSource[1]);
											normalOffset = offs;
											++inputCount;
										}
										else if (strcmp(XMLCHAR2CHAR(prop), "TEXCOORD") == 0) {
											texCoordId = XMLCHAR2CHAR(&propSource[1]);
											texCoordOffset = offs;
											++inputCount;
										}
										else if (strcmp(XMLCHAR2CHAR(prop), "COLOR") == 0) {
											colorId = XMLCHAR2CHAR(&propSource[1]);
											colorOffset = offs;
											++inputCount;
										}

										XMLCHAR_SAFEFREE(propSource);
										XMLCHAR_SAFEFREE(propOffset);
									}

									xmlFree(prop);
								}
								child = getNextNode(child, "input");
							}

							child = getFirstChildNode(triangleChild, "p");
							if (child) {
								child = child->children;
								if (child->type == XML_TEXT_NODE)
									parseIntArray(child, model->dataIndex);
							}
						}

						model->vertexOffset = vertexOffset;
						model->normalOffset = normalOffset;
						model->texCoordOffset = texCoordOffset;
						model->colorOffset = colorOffset;
						model->inputCount = inputCount;
						readVertices(meshNode, model);
						if (*normalId.c_str())
							readNormals(meshNode, normalId.c_str(), model);
						if (*texCoordId.c_str(), model)
							readTextureCoords(meshNode, texCoordId.c_str(), model);
						if (*colorId.c_str())
							readColors(meshNode, colorId.c_str(), model);
					}

					m_models.push_back(model);
				} while (node = getNextNode(node, "geometry"));
			}
		}
	}

	int findDuplicateVertex(std::vector<Vector3> &vertices, Vector3 vertex) {
		for (unsigned int i = 0; i < vertices.size(); ++i) {
			if (vertices[i] == vertex)
				return i;
		}
		return -1;
	}

	int ColladaParser::assembleModel(ParsingModel *model) {

		std::vector<Vector3> vertices;
		std::vector<int> indices;
		std::vector<Vector3> normals;
		std::vector<Color> colors;
		std::vector<bool> bools;

		unsigned int i;

		int count = model->dataIndex.size() / model->inputCount;
		int vertOffs = model->vertexOffset;
		int normalOffs = model->normalOffset;
		int texCoordOffs = model->texCoordOffset;
		int colorOffs = model->colorOffset;

		if (vertOffs >= 0) {
			for (i = 0; i < model->vertices.size()/3; ++i) {
				Vector3 vertex;
				vertex.setX(model->vertices[i * 3 + 0]);
				vertex.setY(model->vertices[i * 3 + 2]);
				vertex.setZ(-model->vertices[i * 3 + 1]);
				bools.push_back(false);
				vertices.push_back(vertex);
				normals.push_back(Vector3(0, 0, 0));
			}

			for (i = 0; i < count; ++i) {

				//int texCoordIndex = model->dataIndex[i*model->inputCount + texCoordOffs];
				

				if (normalOffs >= 0) {
					int vertIndex = model->dataIndex[i*model->inputCount + vertOffs];
					int normalIndex = model->dataIndex[i*model->inputCount + normalOffs];

					float vertX = model->vertices[vertIndex * 3];
					float vertY = model->vertices[vertIndex * 3 + 2];
					float vertZ = -model->vertices[vertIndex * 3 + 1];

					float normalX = model->normals[normalIndex * 3];
					float normalY = model->normals[normalIndex * 3 + 2];
					float normalZ = model->normals[normalIndex * 3 + 1];

					Vector3 vertex = Vector3(vertX, vertY, vertZ);
					Vector3 normal = Vector3(normalX, normalY, normalZ);

					if (!bools[vertIndex]) {
						normals[vertIndex] = normal;
						indices.push_back(vertIndex);
						bools[vertIndex] = true;
					}
					else {
						if (normals[vertIndex] == normal) {
							indices.push_back(vertIndex);
						}
						else {
							vertices.push_back(vertex);
							normals.push_back(normal);
							bools.push_back(true);
							indices.push_back(vertices.size() - 1);
						}
					}
				}

				if (colorOffs >= 0) {
					int colorIndex = model->dataIndex[i*model->inputCount + colorOffs];
					float colorR = model->colors[colorIndex * 3];
					float colorG = model->colors[colorIndex * 3 + 1];
					float colorB = model->colors[colorIndex * 3 + 2];
					Color color = Color(colorR, colorG, colorB);
					colors.push_back(color);
				}
				/*
				int duplicateIndex = findDuplicateVertex(vertices,vertex);
				if(duplicateIndex < 0) {
					//different vertex/normal/texcoord
					vertices.push_back(vertex);
					normals.push_back(normal);
					indices.push_back(vertIndex);
				}
				else {
					if (normals[duplicateIndex] == normal) {
						indices.push_back(duplicateIndex);
					}
					else {
						vertices.push_back(vertex);
						normals.push_back(normal);
						indices.push_back(vertices.size()-1);
					}
				}
				*/
				
			}
		}

		int indicesCount = indices.size();
		
		Vector3 *outVertices = new Vector3[vertices.size()];
		Vector3 *outNormals = new Vector3[vertices.size()];
		
		int *outIndices = new int[indices.size()];
		for (i = 0; i < vertices.size(); ++i) {
			outVertices[i] = vertices[i];
			outNormals[i] = normals[i];
		}
		for (i = 0; i < indices.size(); ++i) {
			outIndices[i] = indices[i];
		}

		int resId = m_engine.newResource(vertices.size(), outVertices, indices.size(), outIndices);
		ResourceData *data = m_engine.resourceData(resId);
		data->setNormals(outNormals);
		if (colors.size() > 0) {
			Color *outColors = new Color[colors.size()];
			for (i = 0; i < colors.size(); ++i) {
				outColors[i] = colors[i];
			}
			data->setColors(outColors);
			delete[] outColors;
		}

		delete[] outVertices;
		delete[] outNormals;
		delete[] outIndices;			

		return resId;
	}


	ColladaParser::ColladaParser(RenderEngine &engine)
		: m_engine(engine) {

	}

	std::vector<int> ColladaParser::parse(const char *fileName)
	{
		std::vector<int> resIds;

		//init
		xmlDocPtr doc = NULL;
		xmlNodePtr rootElement = NULL;

		doc = xmlReadFile(fileName, NULL, 0);
		if (doc == NULL) {
			//	printf("Error: could not parse file %s\n", fullXMLFilePath);
			return std::vector<int>();
		}

		rootElement = xmlDocGetRootElement(doc);
		if (rootElement == NULL) {
			printf("%s File is empty.\n", fileName);
		}
		else {
			if (rootElement->type != XML_ELEMENT_NODE || xmlStrcmp(rootElement->name, XMLCHAR("COLLADA")) != 0) {
				//invalid file
				return std::vector<int>();
			}

			readModel(rootElement);

			for (unsigned int i = 0; i < m_models.size(); ++i)
				resIds.push_back(assembleModel(m_models[i]));

			xmlFreeDoc(doc);
		}

		return resIds;
	}
}
