#ifndef _NODE_H_
#define _NODE_H_

#include "GLM/glm.hpp"
#include "Assimp/scene.h"

#include <vector>
#include <string>

class Node {
public:
	std::string nodeName;
	glm::mat4 model;
	std::vector<int> meshID; //stores the id of meshes in this node
	std::vector<Node*> children;

	Node();
	Node(aiNode* node);
	~Node();

	//find the first node that has the argument nodeName starting from this node
	Node* find(std::string& nodeName);

private:
	//the initial transformation model
	glm::mat4 initialModel;
};

#endif
