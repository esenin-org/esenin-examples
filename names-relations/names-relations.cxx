#include "esenin.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

void print_path(const std::vector<std::string> &tokens, 
                const std::vector<int> &path, 
                const std::vector<int> &entity1, 
                const std::vector<int> &entity2) 
{
  for (int i : entity1) {
    std::cout << tokens[i] << " ";
  }  

  std::cout << "| ";

  for (int i : path) {
    std::cout << tokens[i] << " ";
  }

  std::cout << "| ";

  for (int i : entity2) {
    std::cout << tokens[i] << " ";
  } 
  std::cout << std::endl;  
}

std::vector<int> get_dependency_path(const std::vector<int> &idxes1, 
                                     const std::vector<int> &idxes2, 
                                     const std::vector<int> &parents)
{
  std::set <int> all_parents1;
  std::set <int> all_parents2;

  for (int idx : idxes1) {
    idx = parents[idx];
    while (idx != -1) {
      all_parents1.insert(idx);
      idx = parents[idx];
    }    
  }

  for (int idx : idxes2) {
    idx = parents[idx];
    while (idx != -1) {
      all_parents2.insert(idx);
      idx = parents[idx];
    }    
  }

  std::set<int> intersect;
  std::set_intersection(all_parents1.begin(), 
                        all_parents1.end(),
                        all_parents2.begin(),
                        all_parents2.end(),
                        std::inserter(intersect, intersect.begin()));

  while (true) {
    bool found = false;
    std::vector<int> old_intersect(intersect.begin(), intersect.end());
    for (int idx : old_intersect) {
      auto idx_parent = intersect.find(parents[idx]);
      if (idx_parent != intersect.end()) {
        found = true;
        intersect.erase(idx_parent);
      }
    }
    if (!found)
      break;
  }

  std::set<int> path;

  path.insert(intersect.begin(), intersect.end());

  for (int idx : idxes1) {
    idx = parents[idx];
    while (idx != -1 && intersect.find(idx) == intersect.end()) {
      path.insert(idx);
      idx = parents[idx];
    }
  }

  for (int idx : idxes2) {
    idx = parents[idx];
    while (idx != -1 && intersect.find(idx) == intersect.end()) {
      path.insert(idx);
      idx = parents[idx];
    }
  }

  for (int idx : idxes1)
      path.erase(idx);
  for (int idx : idxes2)
      path.erase(idx);

  return std::vector<int>(path.begin(), path.end());
}

int main(int argc, char *argv[])
{
  Client client = Client(std::string("127.0.0.1"), 9000L);

  std::ifstream infile("lenta-ru-news-texts.txt");
  std::string line;

  while (std::getline(infile, line))
  {    
      std::vector<std::string> sentences = client.sentenize(line);
      for (const std::string& sentence : sentences) {                
          std::vector<std::string> tokens = client.tokenize(sentence);        
          std::vector<NamedEntity> entities = client.get_named_entities(tokens);        
          std::vector<DependencyTreeNode> nodes = client.get_dependency_tree(tokens);

          std::vector<int> parents;
          parents.reserve(nodes.size());
          for (const auto &n : nodes) {
            parents.emplace_back(n.parent);
          }
        
          std::vector<std::vector<int>> entities1, entities2, found_paths;

          for (size_t i = 0; i < entities.size(); i++) {
            for (size_t j = i + 1; j < entities.size(); j++) {          
              std::vector<int> path = get_dependency_path(entities[i].indexes, entities[j].indexes, parents);
              if (path.size() != 0) {       
                entities1.emplace_back(entities[i].indexes);
                entities2.emplace_back(entities[j].indexes);     
                found_paths.emplace_back(path);
              }
            }
          }

          if (found_paths.size() != 0) {
            std::cout << sentence << std::endl;
            for (size_t i = 0; i < found_paths.size(); i++) {
              print_path(tokens, found_paths[i], entities1[i], entities2[i]);
            }
            std::cout << std::endl;
          }
      }
  }

  return 0;
}