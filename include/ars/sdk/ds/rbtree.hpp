/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file rbtree.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

namespace ars {
    
namespace sdk {

//#pragma pack(push, sizeof(long))
struct rbtree_node_t
{
	struct rbtree_node_t* left;
	struct rbtree_node_t* right;
	struct rbtree_node_t* parent;
	unsigned char color;
};
//#pragma pack(pop)

struct rbtree_root_t
{
	struct rbtree_node_t* node;
};

#define rbtree_entry(ptr, type, member) \
	(type*)((char*)(ptr) - (ptrdiff_t)&(((type*)0)->member))

/// re-banlance rb-tree(rbtree_link node before)
/// @param[in] root rbtree root node
/// @param[in] parent parent node
/// @param[in] link parent left or right child node address
/// @param[in] node insert node(new node)
void rbtree_insert(struct rbtree_root_t* root, struct rbtree_node_t* parent, struct rbtree_node_t** link, struct rbtree_node_t* node);

/// re-banlance rb-tree(rbtree_link node before)
/// @param[in] root rbtree root node
/// @param[in] node rbtree new node
void rbtree_delete(struct rbtree_root_t* root, struct rbtree_node_t* node);

const struct rbtree_node_t* rbtree_first(const struct rbtree_root_t* root);
const struct rbtree_node_t* rbtree_last(const struct rbtree_root_t* root);
const struct rbtree_node_t* rbtree_prev(const struct rbtree_node_t* node);
const struct rbtree_node_t* rbtree_next(const struct rbtree_node_t* node);

} // namespace sdk

} // namespace ars
