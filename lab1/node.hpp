#pragma once
#include <memory>
#include <vector>
#include <utility>

class Node {
public:
	using Child = std::unique_ptr<Node>;
	virtual void eval() = 0;
	void addChild(Node *child) { 
		children.push_back(std::make_unique<Node>(child) );
	}
	std::vector<Child> children;
};
