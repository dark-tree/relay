
#include "map.h"
#include "set.h"
#include "mutex.h"
#include "logger.h"
#include "sequence.h"
#include "user.h"
#include "group.h"

bool has_no_duplicates(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            if (arr[i] == arr[j]) {
                return false;
            }
        }
    }

    return true;
}

int main() {

	srand(time(NULL) + 42);

	User* user = user_create(1, 42);
	Group* group = group_create(2, 1);

	IdMap* map = idmap_create();

	{
		IdVec vec = idvec_create(16);
	
		int target = 42;
		void* value = &target;
		
		printf(" (1) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		
		for (int i = 0; i < 100; i ++) {
			idvec_put(&vec, value);
			//printf(" (*) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		}
		
		printf(" (2) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		
		idvec_remove(&vec, 5);
		idvec_remove(&vec, 60);
		idvec_remove(&vec, 2);
		
		printf(" (3) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		
		for (int i = 0; i < 4; i ++) {
			idvec_put(&vec, value);
		}
		
		printf(" (4) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		
		idvec_remove(&vec, 99);
		idvec_remove(&vec, 100);
		
		printf(" (5) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		
		for (int i = 0; i < 100; i ++) {
			idvec_put(&vec, value);
		}
		
		printf(" (6) count=%d, size=%d, offset=%d, alloced=%d\n", vec.count, vec.size, vec.offset, vec.capacity);
		
		idvec_free(&vec);
	}

//	{
//		printf("\nMap Test\n");
//		IdMap* map = idmap_create();
//
//		idmap_put(map, 1, 123);
//		idmap_put(map, 123789, 124);
//		idmap_put(map, 1, 100);
//		idmap_put(map, 50, 5);
//
//		idmap_remove(map, 50);
//
//		printf("Foo: %d\n", idmap_get(map, 123789)->pair.val);
//
//		IdMapIterator iter = idmap_iterator(map);
//
//		printf("Map contents:\n");
//		while (idmap_has(&iter)) {
//			IdMapNode* node = idmap_next(&iter);
//			printf(" * key '%d' equals '%d' \n", node->pair.key, node->pair.val);
//		}
//
//		idmap_free(map);
//	}

//	{
//		printf("\nSet Test\n");
//		IdSet* set = idset_create();
//
//		idset_put(set, 5);
//		idset_put(set, 8);
//		idset_put(set, 9);
//		idset_put(set, 12);
//
//		IdSetIterator iter = idset_iterator(set);
//
//		printf("Set contents:\n");
//		while (idset_has(&iter)) {
//			IdSetNode* node = idset_next(&iter);
//			printf(" * element '%d' \n", node->val);
//		}
//	}

//	{
//		IdSequence seq;
//
//		printf("\nMonotonic Sequence:\n");
//		seq = idseq_begin(IDSEQ_MONOTONIC);
//
//		for (int i = 0; i < 10; i ++) {
//			printf(" * element '%u' \n", idseq_next(&seq));
//		}
//
//		printf("\nRandomized Sequence:\n");
//		seq = idseq_begin(IDSEQ_RANDOMIZED);
//
//		for (int i = 0; i < 10; i ++) {
//			printf(" * element '%u' \n", idseq_next(&seq));
//		}
//
//		// test uniqness
//
//		printf("\nChecking Uniqness:\n");
//		seq = idseq_begin(IDSEQ_RANDOMIZED);
//		int buffer[100000];
//
//		printf(" * generating...\n");
//		for (int i = 0; i < 100000; i ++) {
//			buffer[i] = idseq_next(&seq);
//		}
//
//		printf(" * checking...\n");
//		if (has_no_duplicates(buffer, 10000)) {
//			printf("First 100000 elements are unique!\n");
//		} else {
//			printf("First 100000 elements are NOT unique!\n");
//		}
//
//	}

//	{
//		printf("\n");
//		log_info("Logger Test: %s\n", "PASSED");
//	}

}
