/* WestChamber Windows
 * Elysion
 * March 16 2010
 */

#pragma once

struct avl_node {
	unsigned short value;
	struct avl_node *avl_left;
	struct avl_node *avl_right;
	struct avl_node *next;
	unsigned char avl_height;
};
void avl_insert(struct avl_node * new_node, struct avl_node ** ptree);
struct avl_node* avl_create(unsigned short val);
struct avl_node* avl_search(struct avl_node* tree,unsigned short val);
void avl_delete(struct avl_node* node);