LevelNode:
* This represents a node at a particular level in the LSM tree.
* It contains a list of entries, where each entry is a key-value pair.
* It has a maximum size, which, when exceeded, causes the node to "overflow" and pass its data to the next level.
Level:
* Represents a single level in the LSM tree.
* Contains multiple LevelNode instances.
* Each level can overflow to a "nextLevel", which is another instance of Level.
Root:
* This is the top level of the LSM tree.
* It initially holds data in a linked list format, where new inserts occur at the head.
* If the linked list exceeds its maximum size, it "flushes" its data into the next level of the tree.
LSM:
* The top-level object managing the LSM tree.
* Provides methods to insert, delete, and lookup values based on their keys.

Key Operations
1. Insert:
    * New data is initially inserted into the Root's linked list.
    * If the linked list reaches its maximum size, it flushes the data down into the next level.
    * At lower levels (LevelNode), data is added until it reaches its capacity, after which it is redistributed to the next level down.
2. Delete:
    * Deleting a key involves inserting a special value (TOMBSTONE) for the key, which effectively marks it as deleted.
    * Any future lookups for this key will return the special value indicating that the key is deleted.
3. Lookup:
    * Starts by searching the linked list in the Root.
    * If the key is not found in the linked list, the search continues through the levels of the LSM tree, starting from the top and moving downwards.

