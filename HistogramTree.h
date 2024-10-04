//
// Created by 19210 on 2024/10/3.
//

#ifndef HISTOGRAM_TREE_HISTOGRAMTREE_H
#define HISTOGRAM_TREE_HISTOGRAMTREE_H
#include <vector>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <iostream>

#define FLAG (uint32_t)(1 << 31)

template <typename keyType>
class HistogramTree {
private:
    keyType min;
    keyType max;
    uint32_t dataSize;
    int prefixLength;
    int bins;//一个node的bin的数量
    int width;//sizeof(keyType) * 8 = prefixLength + width
    int threshold;//binSize的阈值
    std::vector<uint32_t> internalNode;
    std::vector<uint32_t> leafNode;
    std::vector<keyType> data;
public:
    HistogramTree()=delete;
    explicit HistogramTree(std::vector<keyType> data, int prefixLength=2, int threshold=16) {
        min = *data.begin();
        max = data.back();
        this->dataSize = data.size();
        this->prefixLength = prefixLength;
        this->bins = 1 << prefixLength;
        this->width = sizeof(keyType) * 8 - prefixLength;
        this->threshold = threshold;
        this->data = data;
        build(data);
    }
    keyType getMin() { return min; }
    keyType getMax() { return max; }
    keyType getBins() { return bins; }
private:
    void build(std::vector<keyType> data) { buildChildNode(data.begin(), data.end(), width);}

    uint32_t buildChildNode(typename std::vector<keyType>::iterator beginIter, typename std::vector<keyType>::iterator endIter, int width) {
        uint64_t binSize = 1 << width;
        std::vector<uint32_t> newBins;
        auto iter = beginIter;
        for (int i = 0; i < bins; ++i) { //初始化newBins
            keyType end = *(beginIter) + (i + 1) * binSize;
            auto NextIter = std::lower_bound(iter, endIter, end);
            uint32_t n = NextIter - iter;
            newBins.push_back(n);
            iter = NextIter;
        }
        assert(iter == endIter);
        bool transferToLeafNode = true;
        for (auto i :newBins) {
            if (i > threshold) {//含有non-terminal bin
                transferToLeafNode = false;
                break;
            }
        }
        //transferToLeafNode
        if (transferToLeafNode) {
            leafNode.insert(leafNode.end(), newBins.begin(), newBins.end());
            return (leafNode.size() / bins - 1) | (uint32_t)FLAG;//返回child Pointer
        }
        //transferToInternalNode
        internalNode.insert(internalNode.end(), newBins.begin(), newBins.end());
        int index = internalNode.size();
        internalNode.insert(internalNode.end(), newBins.begin(), newBins.end());
        uint32_t offset = internalNode.size() / bins - 2;
        uint32_t count = 0;
        for (auto i = 0; i < newBins.size(); ++i) {
            if (newBins[i] > threshold) {//non-terminal bin 需构建孩子节点
                uint32_t offset = buildChildNode(beginIter + count, beginIter + count + newBins[i], width - prefixLength);
                internalNode[index + i] = offset;
            } else {//terminal bin
                internalNode[index + i] = (uint32_t)FLAG;
            }
            count += newBins[i];
        }
        return offset;
    }
public:
    uint32_t lookup(keyType key) {
        auto pos = lookupPredict(key);
        auto beginIter = data.begin() + pos;
        if (beginIter >= data.end()) {
            return pos;
        }
        auto endIter = data.begin() + pos + threshold;
        if (endIter >= data.end()) {
            endIter = data.end();
        }
        return std::lower_bound(beginIter, endIter, key) - data.begin();
    }
    uint32_t lookupPredict(keyType key) {
        if (key < min) {
            return 0;
        }
        if (key > max) {
            return dataSize;
        }

        int tmpWidth = width, done = 0;
        uint32_t i, bin, pos = 0;
        uint32_t next, *node;
        if (internalNode.empty()) {
            node = &leafNode[0];
        } else {
            node = &internalNode[0];
        }
        key -= min;
        do {
            bin = key >> tmpWidth;
            for (i = 0; i < bin; ++i) {
                pos += node[i];
            }
            if (node[bin] <= threshold) {//为terminal bin 直接 返回pos
                return pos;
            }
            done = node[bin + bins] & FLAG;//done 为1 指向leaf Node, done 为 0指向internal Node
            next = node[bin + bins] & (~FLAG);
            node = done ? &leafNode[next * bins] : &internalNode[next * bins];
            key -= bin << tmpWidth;
            tmpWidth -= prefixLength;
        }while(true);
    }
    void print() {
        std::cout << "internalNode: " << internalNode.size() << std::endl;
        auto index = 0;
        for (auto i = 0; i < internalNode.size(); i += 2 * bins) {
            std::cout << "offset: " << index << std::endl;
            index += 2;
            for (auto j = 0; j < bins; ++j) {
                std::cout << internalNode[i + j] << "         ";
            }
            std::cout << std::endl;
            for (auto j = bins; j < 2 * bins; ++j) {
                if (internalNode[i + j] & (uint32_t)FLAG) {
                    uint32_t res = internalNode[i + j] & (~FLAG);
                    if (internalNode[i + j - bins] <= threshold) {
                        std::cout << "terminal  ";
                    } else {
                        std::cout << res << "leaf  ";
                    }
                } else {
                    std::cout << internalNode[i + j] << "         ";
                }
            }
            std::cout << "\n" << std::endl;
        }
        std::cout << "leafNode: " << leafNode.size() << std::endl;
        index = 0;
        for (auto i = 0; i < leafNode.size(); i += bins) {
            std::cout << "offset: " << index << std::endl;
            index += 1;
            for (auto j = 0; j < bins; ++j) {
                std::cout << leafNode[i + j] << "         ";
            }
            std::cout << "\n" << std::endl;
        }
    }
};


#endif //HISTOGRAM_TREE_HISTOGRAMTREE_H
