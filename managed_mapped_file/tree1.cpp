// basic / simple  treeserve builder for normal memory
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cstdint>
#include <unordered_map>
#include <sstream>
#include <utility>
#include <stack>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>

class TreeNode {

  public :

    TreeNode(std::string n, TreeNode *p=0) : name(n), parent(p), size(0.0), children() {
      if (parent != 0) {
        depth=(parent->depth)+1;
        parent->addChild(this);
      } else {
        depth=0;
      }
    }

    std::string getName() {
      return name;
    }
        
    void  incrSize(uint64_t s) {
      size += s;
    }    

    void addChild(TreeNode* c) {
      children.insert(std::make_pair(c->getName(),c));
    }

    TreeNode* getChild(std::string n) {
      std::unordered_map< std::string, TreeNode* >::const_iterator got = children.find(n);
      if (got==children.end()) {
        return 0;
      } else {
        return (*got).second;
      }
    }

    std::string getPath() {
      // this recurses up the parent links to construct the full path to a node
      // don't want to store the full path in the node as that will increase the memory requirements
      // the only time we really need this is when we output the json
      // if we are only outputting 2 or 3 levels deep then the cost overhead in terms of cpu
      // will probably be worth it in terms of the amount of memory saved
      // can revisit this after seeing how things go....
            
      // stack to store the path fragments
      std::stack<std::string> stck;
      TreeNode *curr=this;
      do {
        stck.push(curr->name);
        curr=curr->parent;
      } while (curr != 0);
      std::string tmp="";
      while (!stck.empty()) {
        tmp += "/";
        tmp += stck.top();
        stck.pop();
      }
      return tmp;
    }
        
    std::string toJSON(uint64_t d, uint64_t s=0) {
      std::stringstream oss;
      std::string space="";
      for (int i=0; i<s; i++) {
        space+="  ";
      }
      ++s;
      oss << space << "{" << std::endl;
      oss << space << "\"path\" : \"" << getPath() << "\", \"size\" : " << size << std::endl;
      --d;
      if ( d > 0 && (!children.empty()) ) {
        oss << space << "\"children\" : [" << std::endl;
        bool sep=false;
        std::unordered_map< std::string, TreeNode* >::iterator it;
        for (it=children.begin(); it != children.end(); it++) {
          if (sep) {
            oss << space << "," << std::endl;;
            sep=true;
          }
          oss << ((*it).second)->toJSON(d,s);
          sep=",";
        }
        oss << space << "]" << std::endl;
      }
      oss << space << "}" << std::endl;
      return oss.str();       
    }

    // adds a *.* to the children of a node if the sum of sizes of children is less than the size
    // of the dir itself
    void finalize() {
      // only finalize if the node has children
      if (!children.empty()) {
        // loop over children, sum their sizes and call finalize on them
        uint64_t childSize=0;
        std::unordered_map< std::string, TreeNode* >::iterator it;
        for (it=children.begin(); it != children.end(); it++) {
          childSize += ((*it).second)->size;
          ((*it).second)->finalize();
        }
        // once returned from finalizing children, finalize this node
        uint64_t mySize=size-childSize;
        if (mySize > 0) {
          TreeNode *child=new TreeNode("*.*",this);
          addChild(child);
          child->incrSize(size-childSize);
        }
      }
    }
    
    private:
      std::string name;
      TreeNode *parent;   
      uint64_t size;
      std::unordered_map<std::string,TreeNode*> children;
      uint64_t depth;
};

class Tree {
    
  public :

    Tree() : root(0) {}

    ~Tree() {delete root;}
        
    void addNode(std::string path, double size) {
      // path will be a string of form (/)a/b/c/d(/)
      // need to create any nodes that don't exist
      // e.g. for the above path, if we are adding to an empty tree
      // will need to create the 'a' node, then create 'b' as a child, then 'c'
      // as a child of 'b' then add the leaf 'd' as a child of 'c'
      // also need to increment the size on each node as we descend down
      // we may not need to actually make any nodes if all the ones in the path exist,
      // but we need to increment the size on each node in the path
            
      // turn the path into a vector of names
      std::vector<std::string> names;
      boost::trim_if(path, boost::is_any_of("/"));
      boost::split(names, path, boost::is_any_of("/"));            
      if (root==0) {
        root=new TreeNode(names[0]);
      }
      TreeNode *current=root;
      std::vector<std::string>::iterator it=names.begin();
      ++it;
      for (;it<names.end();it++) {
        current->incrSize(size);
        TreeNode *tmp=current->getChild(*it);
        if (tmp == 0) {
          current=new TreeNode(*it,current);
        } else {
          current=tmp;
        }
      }
      current->incrSize(size);
    }

    TreeNode* getNodeAt(std::string path) {
      // turn the path into a vector of names
      std::vector<std::string> names;
      boost::trim_if(path, boost::is_any_of("/"));
      boost::split(names, path, boost::is_any_of("/"));
      TreeNode *current=root;
      std::vector<std::string>::iterator it=names.begin();
      ++it;
      for (;it<names.end();it++) {
        current=current->getChild(*it);
        if (current==0) {
          return 0;
        }
      }
      return current;
    }

    TreeNode* getRoot() {
      return root;
    }
        
    // once we've finished a tree, want to add a child to each node to represent *.*
    // i.e. size of files within the directory itself. this will be calculated by
    // summing the sizes of all children and subtracting from the size of the node
    void finalize() {
      root->finalize();
    }
        
    std::string toJSON(std::string path, uint64_t d=std::numeric_limits<uint64_t>::max()) {
      if (d==0) d=1;
        TreeNode *tmp=getNodeAt(path);
        return tmp->toJSON(d,0);
      }
      std::string toJSON(uint64_t d) {
      if (d==0) d=1;
        return root->toJSON(d,0);
      }
      std::string toJSON() {
      return root->toJSON(std::numeric_limits<uint64_t>::max(),0);
    }    

  private:
    TreeNode *root;
};

/////////////////////////////////
int main(int argc, char **argv) {
/////////////////////////////////

  // commandline arguments - first is mpistat file

  // get commandline arguments

  // open mpistat (assumed to be gzipped)

  // read lines, and parse them to get path, inode type and size

  // create treenode object to represent the inode

  // attach the treenode at the right point in the tree

  // when tree is finished, do a depth-first traversal to accumulate all the sizes

  // print json of tree up to depth N


  Tree tree;
  tree.addNode("/a/b/c/d/e",123);
  tree.addNode("/a/b/c/d/f",345);
  tree.addNode("/a/b/c/g/h",534);
  std::cout << tree.toJSON() << std::endl; 
  
  // exit
  return 0;
}
