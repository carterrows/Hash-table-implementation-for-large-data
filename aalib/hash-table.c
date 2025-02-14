#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hashtools.h"

#define SEARCH 0
#define DELETE 0
#define INSERT 1

/** forward declaration */
static HashAlgorithm lookupNamedHashStrategy(const char *name);
static HashProbe lookupNamedProbingStrategy(const char *name);

/**
 * Create a hash table of the given size,
 * which will use the given algorithm to create hash values,
 * and the given probing strategy
 *
 *  @param  hash  the HashAlgorithm to use
 *  @param  probingStrategy algorithm used for probing in the case of
 *				collisions
 *  @param  newHashSize  the size of the table (will be rounded up
 *				to the next-nearest larger prime, but see exception)
 *  @see         HashAlgorithm
 *  @see         HashProbe
 *  @see         Primes
 *
 *  @throws java.lang.IndexOutOfBoundsException if no prime number larger
 *				than newHashSize can be found (currently only primes
 *				less than 5000 are known)
 */
AssociativeArray *
aaCreateAssociativeArray(
		size_t size,
		char *probingStrategy,
		char *hashPrimary,
		char *hashSecondary
	)
{
	AssociativeArray *newTable;

	newTable = (AssociativeArray *) malloc(sizeof(AssociativeArray));

	newTable->hashAlgorithmPrimary = lookupNamedHashStrategy(hashPrimary);
	newTable->hashNamePrimary = strdup(hashPrimary);
	newTable->hashAlgorithmSecondary = lookupNamedHashStrategy(hashSecondary);
	newTable->hashNameSecondary = strdup(hashSecondary);
	newTable->hashProbe = lookupNamedProbingStrategy(probingStrategy);
	newTable->probeName = strdup(probingStrategy);

	newTable->size = getLargerPrime(size);

	if (newTable->size < 1) {
		fprintf(stderr, "Cannot create table of size %ld\n", size);
		free(newTable);
		return NULL;
	}

	newTable->table = (KeyDataPair *) malloc(newTable->size * sizeof(KeyDataPair));

	/** initialize everything with zeros */
	memset(newTable->table, 0, newTable->size * sizeof(KeyDataPair));

	newTable->nEntries = 0;

	newTable->insertCost = newTable->searchCost = newTable->deleteCost = 0;

	return newTable;
}

/**
 * deallocate all the memory in the store -- the keys (which we allocated),
 * and the store itself.
 * The user * code is responsible for managing the memory for the values
 */
void
aaDeleteAssociativeArray(AssociativeArray *aarray)
{
	/**
	 * TO DO:  clean up the memory managed by our utility
	 *
	 * Note that memory for keys are managed, values are the
	 * responsibility of the user
	 */

	// handle a NULL pointer
	if(aarray == NULL) {
		return;
	}

	if(aarray->table != NULL) {
		for(int i = 0; i < aarray->size; i++) {
			KeyDataPair *temp = &aarray->table[i];
			if(temp->validity == HASH_USED || temp->validity == HASH_DELETED) {
				free(temp->key);
			}
		}
		free(aarray->table);
	}

	free(aarray->probeName);
	free(aarray->hashNamePrimary);
	free(aarray->hashNameSecondary);
	free(aarray);
}

/**
 * iterate over the array, calling the user function on each valid value
 */
int aaIterateAction(
		AssociativeArray *aarray,
		int (*userfunction)(AAKeyType key, size_t keylen, void *datavalue, void *userdata),
		void *userdata)
{
	int i;

	for (i = 0; i < aarray->size; i++) {
		if (aarray->table[i].validity == HASH_USED) {
			if ((*userfunction)(
					aarray->table[i].key,
					aarray->table[i].keylen,
					aarray->table[i].value,
					userdata) < 0) {
				return -1;
			}
		}
	}
	return 1;
}

/** utilities to change names into functions, used in the function above */
static HashAlgorithm lookupNamedHashStrategy(const char *name)
{
	if (strncmp(name, "sum", 3) == 0) {
		return hashBySum;
	} else if (strncmp(name, "len", 3) == 0) {
		return hashByLength;

		// TO DO: add in your own strategy here
	} else if (strncmp(name, "car", 3) == 0) {
		return hash_CARTER;
	}

	fprintf(stderr, "Invalid hash strategy '%s' - using 'sum'\n", name);
	return hashBySum;
}

static HashProbe lookupNamedProbingStrategy(const char *name)
{
	if (strncmp(name, "lin", 3) == 0) {
		return linearProbe;
	} else if (strncmp(name, "qua", 3) == 0) {
		return quadraticProbe;
	} else if (strncmp(name, "dou", 3) == 0) {
		return doubleHashProbe;
	}

	fprintf(stderr, "Invalid hash probe strategy '%s' - using 'linear'\n", name);
	return linearProbe;
}

/**
 * Add another key and data value to the table, provided there is room.
 *
 *  @param  key  a string value used for searching later
 *  @param  value a data value associated with the key
 *  @return      the location the data is placed within the hash table,
 *				 or a negative number if no place can be found
 */
int aaInsert(AssociativeArray *aarray, AAKeyType key, size_t keylen, void *value)
{
	/**
	 * TO DO:  Search for a location where this key can go, stopping
	 * if we find a value that has been delete and reuse it.
	 *
	 * If a suitable location is found, we then initialize that
	 * slot with the new key and data
	 */

	// calculate index from hash
	HashIndex startIndex = aarray->hashAlgorithmPrimary(key, keylen, aarray->size);

	// probe using given probing strategy
	HashIndex insertIndex = aarray->hashProbe(aarray, key, keylen, startIndex, INSERT, &aarray->insertCost);

	// check if table is full or we found duplicate key
	if(insertIndex < 0) {
		printf("(Carter printed this) Insertion failed for key: '%s'\n", key);
		return -1;
	}

	// fill the KeyDataPair struct
	aarray->table[insertIndex].key = (AAKeyType) malloc(keylen);
	memcpy(aarray->table[insertIndex].key, key, keylen);

	aarray->table[insertIndex].keylen = keylen;
	aarray->table[insertIndex].validity = HASH_USED;
	
	aarray->table[insertIndex].value = value;

	// update number of entries
	aarray->nEntries++;

	return insertIndex;
}


/**
 * Locates the KeyDataPair associated with the given key, if
 * present in the table.
 *
 *  @param  key  the key to search for
 *  @return      the KeyDataPair containing the key, if the key
 *				 was present in the table, or NULL, if it was not
 *  @see         KeyDataPair
 */
void *aaLookup(AssociativeArray *aarray, AAKeyType key, size_t keylen)
{
	/**
	 * TO DO: perform a similar search to the insert, but here a
	 * deleted location means we have not found the key
	 */

	//printf("Carter printed this: Top of aaLookup function.\n");
	// calculate index from hash
	HashIndex startIndex = aarray->hashAlgorithmPrimary(key, keylen, aarray->size);
	//printf("Carter printed this: After finding startIndex.\n");

	// find index of key
	HashIndex checkIndex = aarray->hashProbe(aarray, key, keylen, startIndex, SEARCH, &aarray->searchCost);
	//printf("Carter printed this: After finding checkIndex\n");
	
	// check if we have found the key
	if(checkIndex > 0) {
		return aarray->table[checkIndex].value;
	}

	return NULL;
}


/**
 * Locates the KeyDataPair associated with the given key, if
 * present in the table.
 *
 *  @param  key  the key to search for
 *  @return      the index of the KeyDataPair containing the key,
 *				 if the key was present in the table, or (-1),
 *				 if no key was found
 *  @see         KeyDataPair
 */
void *aaDelete(AssociativeArray *aarray, AAKeyType key, size_t keylen)
{
	/**
	 * TO DO: Deletion is closely related to lookup;
	 * you must find where the key is stored before
	 * you delete it, after all.
	 *
	 * Implement a deletion algorithm based on tombstones,
	 * as described in class
	 */

	// calculate index from hash
	HashIndex startIndex = aarray->hashAlgorithmPrimary(key, keylen, aarray->size);

	// probe using given probing strategy
	HashIndex deleteIndex = aarray->hashProbe(aarray, key, keylen, startIndex, DELETE, &aarray->deleteCost);

	// check if we have found the key
	if(deleteIndex > 0) {
		aarray->table[deleteIndex].validity = HASH_DELETED;
		return aarray->table[deleteIndex].value;
	}

	return NULL;
}

/**
 * Print out the entire aarray contents
 */
void aaPrintContents(FILE *fp, AssociativeArray *aarray, char * tag)
{
	char keybuffer[128];
	int i;

	fprintf(fp, "%sDumping aarray of %d entries:\n", tag, aarray->size);
	for (i = 0; i < aarray->size; i++) {
		fprintf(fp, "%s  ", tag);
		if (aarray->table[i].validity == HASH_USED) {
			printableKey(keybuffer, 128,
					aarray->table[i].key,
					aarray->table[i].keylen);
			fprintf(fp, "%d : in use : '%s'\n", i, keybuffer);
		} else {
			if (aarray->table[i].validity == HASH_EMPTY) {
				fprintf(fp, "%d : empty (NULL)\n", i);
			} else if ( aarray->table[i].validity == HASH_DELETED) {
				printableKey(keybuffer, 128,
						aarray->table[i].key,
						aarray->table[i].keylen);
				fprintf(fp, "%d : empty (deleted - was '%s')\n", i, keybuffer);
			} else {
				fprintf(fp, "%d : invalid validity state %d\n", i,
						aarray->table[i].validity);
			}
		}
	}
}


/**
 * Print out a short summary
 */
void aaPrintSummary(FILE *fp, AssociativeArray *aarray)
{
	fprintf(fp, "Associative array contains %d entries in a table of %d size\n",
			aarray->nEntries, aarray->size);
	fprintf(fp, "Strategies used: '%s' hash, '%s' secondary hash and '%s' probing\n",
			aarray->hashNamePrimary, aarray->hashNameSecondary, aarray->probeName);
	fprintf(fp, "Costs accrued due to probing:\n");
	fprintf(fp, "  Insertion : %d\n", aarray->insertCost);
	fprintf(fp, "  Search    : %d\n", aarray->searchCost);
	fprintf(fp, "  Deletion  : %d\n", aarray->deleteCost);
}

