/***************************************************************************
 *   Copyright (C) 2012-2023 by OpenCPN development team                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/

#ifndef SPATIAL_RTREE_H
#define SPATIAL_RTREE_H

#include <vector>
#include <algorithm>
#include <memory>
#include <limits>
#include <utility>
#include <functional>
#include <ShapefileReader.hpp>

/**
 * A Bounding Box class for spatial indexing
 *
 * Represents a Minimum Bounding Rectangle (MBR) used in the R-tree
 * implementation. It's defined by minimum and maximum coordinates in
 * latitude/longitude space.
 */
class RTreeBBox {
public:
  /**
   * Default constructor creates an invalid box that can be expanded
   */
  RTreeBBox()
      : minLat(std::numeric_limits<double>::max()),
        minLon(std::numeric_limits<double>::max()),
        maxLat(std::numeric_limits<double>::lowest()),
        maxLon(std::numeric_limits<double>::lowest()) {}

  /**
   * Constructor from min/max coordinates
   */
  RTreeBBox(double minLat, double minLon, double maxLat, double maxLon)
      : minLat(minLat), minLon(minLon), maxLat(maxLat), maxLon(maxLon) {}

  /**
   * Creates a bounding box for a polygon feature
   */
  static RTreeBBox FromFeature(const shp::Feature& feature) {
    RTreeBBox bbox;
    auto geometry = feature.getGeometry();
    if (geometry) {
      auto* polygon = dynamic_cast<shp::Polygon*>(geometry);
      if (polygon) {
        for (auto& ring : polygon->getRings()) {
          for (auto& point : ring.getPoints()) {
            bbox.Expand(point.getY(), point.getX());
          }
        }
      }
    }
    return bbox;
  }

  /**
   * Creates a bounding box containing a line segment
   */
  static RTreeBBox FromLineSegment(double lat1, double lon1, double lat2,
                                     double lon2) {
    RTreeBBox bbox;
    bbox.Expand(lat1, lon1);
    bbox.Expand(lat2, lon2);
    return bbox;
  }

  /**
   * Expands the bounding box to include the given point
   */
  void Expand(double lat, double lon) {
    minLat = std::min(minLat, lat);
    minLon = std::min(minLon, lon);
    maxLat = std::max(maxLat, lat);
    maxLon = std::max(maxLon, lon);
  }

  /**
   * Expands the bounding box to include another bounding box
   */
  void Expand(const RTreeBBox& other) {
    if (other.IsValid()) {
      minLat = std::min(minLat, other.minLat);
      minLon = std::min(minLon, other.minLon);
      maxLat = std::max(maxLat, other.maxLat);
      maxLon = std::max(maxLon, other.maxLon);
    }
  }

  /**
   * Checks if this bounding box intersects with another
   */
  bool Intersects(const RTreeBBox& other) const {
    // No intersection if one box is to the left of the other
    if (maxLon < other.minLon || other.maxLon < minLon) return false;

    // No intersection if one box is above the other
    if (minLat > other.maxLat || other.minLat > maxLat) return false;

    // Boxes intersect
    return true;
  }

  /**
   * Checks if this bounding box contains a point
   */
  bool Contains(double lat, double lon) const {
    return lat >= minLat && lat <= maxLat && lon >= minLon && lon <= maxLon;
  }

  /**
   * Checks if this bounding box is valid (has been initialized with points)
   */
  bool IsValid() const {
    return minLat <= maxLat && minLon <= maxLon &&
           minLat != std::numeric_limits<double>::max();
  }

  /**
   * Calculates the area of this bounding box
   */
  double Area() const {
    if (!IsValid()) return 0;
    return (maxLat - minLat) * (maxLon - minLon);
  }

  /**
   * Returns the enlargement area if this box were to include the given box
   */
  double EnlargementArea(const RTreeBBox& other) const {
    if (!IsValid()) return other.Area();
    if (!other.IsValid()) return 0;

    double newMinLat = std::min(minLat, other.minLat);
    double newMinLon = std::min(minLon, other.minLon);
    double newMaxLat = std::max(maxLat, other.maxLat);
    double newMaxLon = std::max(maxLon, other.maxLon);

    double newArea = (newMaxLat - newMinLat) * (newMaxLon - newMinLon);
    return newArea - Area();
  }

  // Minimum/maximum latitude and longitude
  double minLat;
  double minLon;
  double maxLat;
  double maxLon;
};

/**
 * Base class for R-tree nodes
 *
 * Abstract base class for both leaf and internal nodes in the R-tree.
 */
class RTreeNode {
public:
  RTreeNode(bool isLeaf = false) : isLeaf(isLeaf) {}
  virtual ~RTreeNode() = default;

  // Pure virtual methods to be implemented by derived classes
  virtual RTreeBBox GetRTreeBBox() const = 0;
  virtual void Search(const RTreeBBox& queryBox,
                      std::vector<size_t>& results) const = 0;

  bool IsLeaf() const { return isLeaf; }

protected:
  bool isLeaf;
};

/**
 * Leaf node containing actual data entries
 *
 * Contains references to actual spatial objects (polygons, lines, etc.)
 * via their indices in the original data source.
 */
class RTreeLeafNode : public RTreeNode {
public:

  struct Entry {
    size_t index;  // Index of the object in the original data
    RTreeBBox box;
  };

  RTreeLeafNode() : RTreeNode(true) {}

  void AddEntry(size_t index, const RTreeBBox& box) {
    entries.push_back(Entry{index, box});
    nodeRTreeBBox.Expand(box);
  }

  RTreeBBox GetRTreeBBox() const override { return nodeRTreeBBox; }

  void Search(const RTreeBBox& queryBox,
              std::vector<size_t>& results) const override {
    for (const auto& entry : entries) {
      if (entry.box.Intersects(queryBox)) {
        results.push_back(entry.index);
      }
    }
  }

  bool IsFull(size_t maxEntries) const { return entries.size() >= maxEntries; }

  const std::vector<Entry>& GetEntries() const { return entries; }

private:
  std::vector<Entry> entries;
  RTreeBBox nodeRTreeBBox;
};

/**
 * Internal node containing child nodes
 *
 * Each internal node contains pointers to child nodes which can be
 * either leaf nodes or other internal nodes.
 */
class RTreeInternalNode : public RTreeNode {
public:
  RTreeInternalNode() : RTreeNode(false) {}

  void AddChild(std::unique_ptr<RTreeNode> child) {
    nodeRTreeBBox.Expand(child->GetRTreeBBox());
    children.push_back(std::move(child));
  }

  RTreeBBox GetRTreeBBox() const override { return nodeRTreeBBox; }

  void Search(const RTreeBBox& queryBox,
              std::vector<size_t>& results) const override {
    if (!nodeRTreeBBox.Intersects(queryBox)) {
      return;
    }

    for (const auto& child : children) {
      child->Search(queryBox, results);
    }
  }

  bool IsFull(size_t maxEntries) const { return children.size() >= maxEntries; }

  std::unique_ptr<RTreeNode> RemoveChild(size_t index) {
    if (index >= children.size()) return nullptr;

    auto it = children.begin() + index;
    auto child = std::move(*it);
    children.erase(it);

    // Recalculate bounding box
    nodeRTreeBBox = RTreeBBox();
    for (const auto& c : children) {
      nodeRTreeBBox.Expand(c->GetRTreeBBox());
    }

    return child;
  }

  const std::vector<std::unique_ptr<RTreeNode>>& GetChildren() const {
    return children;
  }

private:
  std::vector<std::unique_ptr<RTreeNode>> children;
  RTreeBBox nodeRTreeBBox;
};

/**
 * R-tree spatial index implementation
 *
 * An R-tree implementation for efficient spatial indexing and querying.
 * It organizes spatial data in a hierarchical structure of bounding boxes.
 */
class RTree {
public:
  /**
   * Constructor with specified node capacities
   *
   * @param maxEntries Maximum number of entries in a node before splitting
   * @param minEntries Minimum number of entries in a node (except the root)
   */
  RTree(size_t maxEntries = 8, size_t minEntries = 3)
      : maxEntries(maxEntries), minEntries(minEntries) {
    // Ensure minEntries is at least 2 and at most maxEntries/2
    this->minEntries =
        std::min(std::max(this->minEntries, size_t(2)), this->maxEntries / 2);

    // Create empty root leaf node
    root = std::make_unique<RTreeLeafNode>();
  }

  /**
   * Builds an R-tree from a ShapefileReader
   *
   * @param reader The ShapefileReader containing features to index
   */
  void BuildFromShapefile(const shp::ShapefileReader& reader) {
    size_t index = 0;
    for (const auto& feature : reader) {
      Insert(index, RTreeBBox::FromFeature(feature));
      index++;
    }
  }

  /**
   * Inserts a spatial object into the R-tree
   *
   * @param index Index of the object in the original data source
   * @param box Bounding box of the object
   */
  void Insert(size_t index, const RTreeBBox& box) {
    InsertInternal(root, index, box, 0);
  }

  /**
   * Searches the R-tree for objects that intersect with the query box
   *
   * @param queryBox Bounding box to search with
   * @return Vector of indices of objects that intersect with the query box
   */
  std::vector<size_t> Search(const RTreeBBox& queryBox) const {
    std::vector<size_t> results;
    if (root) {
      root->Search(queryBox, results);
    }
    return results;
  }

  /**
   * Searches for objects that might intersect with a line segment
   *
   * @param lat1 Latitude of the first point
   * @param lon1 Longitude of the first point
   * @param lat2 Latitude of the second point
   * @param lon2 Longitude of the second point
   * @return Vector of indices of objects that might intersect with the line
   */
  std::vector<size_t> SearchLineIntersection(double lat1, double lon1,
                                             double lat2, double lon2) const {
    RTreeBBox queryBox = RTreeBBox::FromLineSegment(lat1, lon1, lat2, lon2);
    return Search(queryBox);
  }

private:
  std::unique_ptr<RTreeNode> root;
  size_t maxEntries;
  size_t minEntries;

  /**
   * Internal recursive insert method
   *
   * @param node Current node to insert into
   * @param index Index of the object in the original data
   * @param box Bounding box of the object
   * @param level Current tree level (0 = leaf)
   * @return New node if the current node was split, nullptr otherwise
   */
  std::unique_ptr<RTreeNode> InsertInternal(std::unique_ptr<RTreeNode>& node,
                                            size_t index,
                                            const RTreeBBox& box, int level) {
    if (node->IsLeaf()) {
      // If we're at a leaf node, add the entry
      auto* leafNode = static_cast<RTreeLeafNode*>(node.get());
      leafNode->AddEntry(index, box);

      // If the node is full, split it
      if (leafNode->IsFull(maxEntries)) {
        return SplitLeafNode(leafNode);
      }
    } else {
      // If we're at an internal node, choose the best child to insert into
      auto* internalNode = static_cast<RTreeInternalNode*>(node.get());
      size_t bestChild = ChooseBestChild(internalNode, box);

      // Insert into the best child
      auto& childNode = internalNode->GetChildren()[bestChild];
      auto newNode = InsertInternal(childNode, index, box, level + 1);

      // If the child was split, add the new node to this node
      if (newNode) {
        internalNode->AddChild(std::move(newNode));

        // If this node is now full, split it
        if (internalNode->IsFull(maxEntries)) {
          return SplitInternalNode(internalNode);
        }
      }
    }

    return nullptr;
  }

  /**
   * Chooses the best child node to insert a new entry
   *
   * @param node The internal node to choose from
   * @param box The bounding box to insert
   * @return Index of the best child node
   */
  size_t ChooseBestChild(const RTreeInternalNode* node,
                         const RTreeBBox& box) const {
    const auto& children = node->GetChildren();
    if (children.empty()) return 0;

    size_t bestChild = 0;
    double minEnlargement = children[0]->GetRTreeBBox().EnlargementArea(box);

    for (size_t i = 1; i < children.size(); i++) {
      double enlargement = children[i]->GetRTreeBBox().EnlargementArea(box);
      if (enlargement < minEnlargement) {
        minEnlargement = enlargement;
        bestChild = i;
      }
    }

    return bestChild;
  }

  /**
   * Splits a leaf node when it exceeds capacity
   *
   * @param node The leaf node to split
   * @return A new leaf node containing some of the entries
   */
  std::unique_ptr<RTreeNode> SplitLeafNode(RTreeLeafNode* node) {
    auto newNode = std::make_unique<RTreeLeafNode>();
    const auto& entries = node->GetEntries();

    // Choose two seeds for the split using the quadratic algorithm
    size_t seed1, seed2;
    ChooseSeeds(entries, seed1, seed2);

    // Distribute entries between the two nodes
    std::vector<bool> assigned(entries.size(), false);
    assigned[seed1] = assigned[seed2] = true;

    // Initialize the nodes with their seeds
    auto oldNodeBox = entries[seed1].box;
    auto newNodeBox = entries[seed2].box;

    // Temporary storage for the original node's entries
    std::vector<RTreeLeafNode::Entry> oldEntries;
    oldEntries.push_back(entries[seed1]);
    newNode->AddEntry(entries[seed2].index, entries[seed2].box);

    // Distribute the remaining entries
    size_t oldCount = 1;  // seed1 already in oldEntries
    size_t newCount = 1;  // seed2 already in newNode

    for (size_t i = 0; i < entries.size(); i++) {
      if (assigned[i]) continue;

      // If one group has too few entries, assign to it
      size_t remaining = entries.size() - oldCount - newCount;
      if (oldCount + remaining <= minEntries) {
        oldEntries.push_back(entries[i]);
        oldNodeBox.Expand(entries[i].box);
        oldCount++;
        continue;
      }

      if (newCount + remaining <= minEntries) {
        newNode->AddEntry(entries[i].index, entries[i].box);
        newNodeBox.Expand(entries[i].box);
        newCount++;
        continue;
      }

      // Otherwise, assign to the group that needs less enlargement
      double oldEnlargement = oldNodeBox.EnlargementArea(entries[i].box);
      double newEnlargement = newNodeBox.EnlargementArea(entries[i].box);

      if (oldEnlargement < newEnlargement ||
          (oldEnlargement == newEnlargement && oldCount < newCount)) {
        oldEntries.push_back(entries[i]);
        oldNodeBox.Expand(entries[i].box);
        oldCount++;
      } else {
        newNode->AddEntry(entries[i].index, entries[i].box);
        newNodeBox.Expand(entries[i].box);
        newCount++;
      }
    }

    // Replace contents of original node with oldEntries
    std::unique_ptr<RTreeNode> originalNode = std::make_unique<RTreeLeafNode>();
    for (const auto& entry : oldEntries) {
      static_cast<RTreeLeafNode*>(originalNode.get())
          ->AddEntry(entry.index, entry.box);
    }

    // Replace the original node with the new one
    auto* leafNode = static_cast<RTreeLeafNode*>(node.get());
    auto* originalLeafNode = static_cast<RTreeLeafNode*>(originalNode.get());
    *leafNode = *originalLeafNode;

    return newNode;
  }

  /**
   * Splits an internal node when it exceeds capacity
   *
   * @param node The internal node to split
   * @return A new internal node containing some of the children
   */
  std::unique_ptr<RTreeNode> SplitInternalNode(RTreeInternalNode* node) {
    auto newNode = std::make_unique<RTreeInternalNode>();
    const auto& children = node->GetChildren();

    // Choose two seeds for the split
    size_t seed1 = 0, seed2 = 1;
    double maxWaste = -1;

    for (size_t i = 0; i < children.size(); i++) {
      for (size_t j = i + 1; j < children.size(); j++) {
        RTreeBBox combinedBox = children[i]->GetRTreeBBox();
        combinedBox.Expand(children[j]->GetRTreeBBox());

        double waste = combinedBox.Area() -
                       children[i]->GetRTreeBBox().Area() -
                       children[j]->GetRTreeBBox().Area();

        if (waste > maxWaste) {
          maxWaste = waste;
          seed1 = i;
          seed2 = j;
        }
      }
    }

    // Distribute children between the two nodes
    std::vector<bool> assigned(children.size(), false);
    assigned[seed1] = assigned[seed2] = true;

    // Initialize nodes with seeds
    RTreeBBox oldNodeBox = children[seed1]->GetRTreeBBox();
    RTreeBBox newNodeBox = children[seed2]->GetRTreeBBox();

    // Temporary storage for original node's children
    std::vector<std::unique_ptr<RTreeNode>> oldChildren;
    oldChildren.push_back(node->RemoveChild(seed1));
    newNode->AddChild(node->RemoveChild(seed2 > seed1 ? seed2 - 1 : seed2));

    // Adjust assigned indices if seed2 was removed first
    if (seed2 < seed1) {
      assigned[seed1 - 1] = true;
      seed1--;
    }

    // Rebuild assigned vector after the removals
    std::vector<bool> newAssigned;
    for (size_t i = 0; i < children.size() + 2; i++) {
      if (i != seed1 && i != seed2) {
        newAssigned.push_back(false);
      }
    }
    assigned = std::move(newAssigned);

    size_t oldCount = 1;  // seed1 already in oldChildren
    size_t newCount = 1;  // seed2 already in newNode

    // Distribute remaining children
    while (!assigned.empty()) {
      // Find the best child to distribute next
      size_t bestChild = 0;
      double bestDiff = std::numeric_limits<double>::max();

      for (size_t i = 0; i < assigned.size(); i++) {
        if (assigned[i]) continue;

        double oldEnlargement = oldNodeBox.EnlargementArea(
            node->GetChildren()[i]->GetRTreeBBox());
        double newEnlargement = newNodeBox.EnlargementArea(
            node->GetChildren()[i]->GetRTreeBBox());

        double diff = std::abs(oldEnlargement - newEnlargement);
        if (diff < bestDiff) {
          bestDiff = diff;
          bestChild = i;
        }
      }

      // Assign the best child
      assigned[bestChild] = true;

      // If one group has too few entries, assign to it
      size_t remaining =
          assigned.size() - std::count(assigned.begin(), assigned.end(), true);
      if (oldCount + remaining <= minEntries) {
        oldChildren.push_back(node->RemoveChild(bestChild));
        oldNodeBox.Expand(oldChildren.back()->GetRTreeBBox());
        oldCount++;
        continue;
      }

      if (newCount + remaining <= minEntries) {
        newNode->AddChild(node->RemoveChild(bestChild));
        newNodeBox.Expand(newNode->GetChildren().back()->GetRTreeBBox());
        newCount++;
        continue;
      }

      // Otherwise, assign to the group that needs less enlargement
      double oldEnlargement = oldNodeBox.EnlargementArea(
          node->GetChildren()[bestChild]->GetRTreeBBox());
      double newEnlargement = newNodeBox.EnlargementArea(
          node->GetChildren()[bestChild]->GetRTreeBBox());

      if (oldEnlargement < newEnlargement ||
          (oldEnlargement == newEnlargement && oldCount < newCount)) {
        oldChildren.push_back(node->RemoveChild(bestChild));
        oldNodeBox.Expand(oldChildren.back()->GetRTreeBBox());
        oldCount++;
      } else {
        newNode->AddChild(node->RemoveChild(bestChild));
        newNodeBox.Expand(newNode->GetChildren().back()->GetRTreeBBox());
        newCount++;
      }

      // Rebuild assigned vector after removal
      std::vector<bool> updatedAssigned;
      size_t assignedIdx = 0;
      for (size_t i = 0; i < assigned.size(); i++) {
        if (i != bestChild) {
          updatedAssigned.push_back(assigned[i]);
        }
      }
      assigned = std::move(updatedAssigned);
    }

    // Replace contents of original node with oldChildren
    std::unique_ptr<RTreeNode> originalNode =
        std::make_unique<RTreeInternalNode>();
    for (auto& child : oldChildren) {
      static_cast<RTreeInternalNode*>(originalNode.get())
          ->AddChild(std::move(child));
    }

    // Replace the original node with the new one
    std::swap(node, static_cast<RTreeInternalNode*>(originalNode.get()));

    return newNode;
  }

  /**
   * Choose seed entries for node splitting using the quadratic method
   *
   * @param entries Vector of entries to choose from
   * @param seed1 Output parameter for the first seed index
   * @param seed2 Output parameter for the second seed index
   */
  template <typename EntryType>
  void ChooseSeeds(const std::vector<EntryType>& entries, size_t& seed1,
                   size_t& seed2) const {
    double maxWaste = -1;

    for (size_t i = 0; i < entries.size(); i++) {
      for (size_t j = i + 1; j < entries.size(); j++) {
        RTreeBBox combined = entries[i].box;
        combined.Expand(entries[j].box);

        double waste =
            combined.Area() - entries[i].box.Area() - entries[j].box.Area();

        if (waste > maxWaste) {
          maxWaste = waste;
          seed1 = i;
          seed2 = j;
        }
      }
    }
  }
};

#endif  // SPATIAL_RTREE_H
