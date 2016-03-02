#include <assert.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


/**
 * Shift structure.
 *   @idx: The index.
 *   @val: The value.
 */

struct shift_t {
	uint64_t idx;
	mpz_t val;
};

/**
 * List structure.
 *   @arr: The array of shifts.
 *   @len: The length.
 */

struct list_t {
	struct shift_t *arr;
	unsigned int len;
};

struct point_t {
	uint64_t idx;
	mpz_t val;
};

struct set_t {
	struct point_t *arr;
	unsigned int len;
};


/*
 * local function declarations
 */

static struct list_t list_init(void);
static void list_destroy(struct list_t *list);
static void list_add(struct list_t *list, uint64_t idx, mpz_t val);
static struct shift_t *list_last(struct list_t *list);
static struct shift_t *list_smaller(struct list_t *list, mpz_t val);
static struct shift_t *list_atmost(struct list_t *list, mpz_t val);

static int64_t *arr_new(unsigned int *len);
static void arr_add(int64_t **arr, unsigned int *len, int64_t idx);

static struct set_t set_init(void);
static void set_destroy(struct set_t *set);
static void set_add(struct set_t *set, uint64_t idx, mpz_t val);


int64_t *proof_enum(mpz_t delta, mpz_t m0, mpz_t alpha, mpz_t tau, unsigned int p)
{
	mpz_t t, v;
	int64_t *arr;
	uint64_t idx;
	unsigned int len;
	struct shift_t *shift;
	struct list_t up, down;

	mpz_inits(t, v, NULL);

	up = list_init();
	down = list_init();

	/* optimal list construction */

	mpz_mod(t, alpha, tau);
	list_add(&up, 1, t);

	mpz_sub(t, t, tau);
	list_add(&down, 1, t);

	while(true) {
		if(list_last(&up)->idx <= list_last(&down)->idx) {
			mpz_sub(t, list_last(&down)->val, list_last(&up)->val);

			shift = list_smaller(&down, t);
			assert(shift != NULL);

			idx = list_last(&up)->idx + shift->idx;
			mpz_add(t, list_last(&up)->val, shift->val);

			if(mpz_sgn(t) >= 0)
				list_add(&up, idx, t);

			if(mpz_sgn(t) <= 0)
				list_add(&down, idx, t);
		}
		else {
			mpz_sub(t, list_last(&up)->val, list_last(&down)->val);

			shift = list_smaller(&up, t);
			assert(shift != NULL);

			idx = list_last(&down)->idx + shift->idx;
			mpz_add(t, list_last(&down)->val, shift->val);

			if(mpz_sgn(t) >= 0)
				list_add(&up, idx, t);

			if(mpz_sgn(t) <= 0)
				list_add(&down, idx, t);

		}

		if((idx >= (1ul << p)) || (mpz_cmp_si(t, 0) == 0))
			break;
	}

	/* search algorithm */

	idx = 0;
	mpz_mod(v, m0, tau);
	mpz_sub(t, v, tau);
	if(mpz_cmpabs(t, v) < 0)
		mpz_set(v, t);

	while(true) {
		if((mpz_cmpabs(v, delta) <= 0) || (idx >= (1ul << p)))
			break;

		if(mpz_sgn(v) < 0) {
			mpz_mul_ui(t, v, 2);

			shift = list_atmost(&up, t);
			if(shift == NULL)
				break;
		}
		else {
			mpz_mul_ui(t, v, 2);

			shift = list_atmost(&down, t);
			if(shift == NULL)
				break;
		}

		idx += shift->idx;
		mpz_add(v, v, shift->val);
	}

	/* exhaustively enumerate */

	arr = arr_new(&len);

	if((idx < (1ul << p)) && (shift != NULL)) {
		int i, j;
		struct set_t set;

		set = set_init();
		set_add(&set, idx, v);

		idx = 0;

		for(i = 0; i < set.len; i++) {
			arr_add(&arr, &len, set.arr[i].idx);

			if(mpz_cmpabs(set.arr[i].val, delta) > 0)
				printf("ERR\n"), abort();

			for(j = up.len - 1; j >= 0; j--) {
				idx = set.arr[i].idx + up.arr[j].idx;
				if(idx >= (1ul << p))
					continue;

				mpz_add(v, set.arr[i].val, up.arr[j].val);
				if(mpz_cmpabs(v, delta) > 0)
					break;

				set_add(&set, idx, v);
			}

			for(j = down.len - 1; j >= 0; j--) {
				idx = set.arr[i].idx + down.arr[j].idx;
				if(idx >= (1ul << p))
					continue;

				mpz_add(v, set.arr[i].val, down.arr[j].val);
				if(mpz_cmpabs(v, delta) > 0)
					break;

				set_add(&set, idx, v);
			}
		}

		set_destroy(&set);
	}

	list_destroy(&up);
	list_destroy(&down);

	mpz_clears(t, v, NULL);

	return arr;
}


/**
 * Initialize an empty shift list.
 *   &returns: The list.
 */

static struct list_t list_init(void)
{
	return (struct list_t){ malloc(0), 0 };
}

/**
 * Destroy a list of shifts.
 *   @list: The list.
 */

static void list_destroy(struct list_t *list)
{
	unsigned int i;

	for(i = 0; i < list->len; i++)
		mpz_clear(list->arr[i].val);

	free(list->arr);
}

/**
 * Add a shift to end of the sorted list.
 *   @list: The list.
 *   @idx: The index.
 *   @val: The value.
 */

static void list_add(struct list_t *list, uint64_t idx, mpz_t val)
{
	unsigned int n = list->len;

	assert((list->len == 0) || (list_last(list)->idx < idx));
	assert((list->len == 0) || (mpz_cmp(list_last(list)->val, val) * mpz_sgn(val) >= 0));

	list->len++;
	list->arr = realloc(list->arr, list->len * sizeof(struct shift_t));
	list->arr[n].idx = idx;
	mpz_init_set(list->arr[n].val, val);
}

/**
 * Retrieve the last shift from the list.
 *   @list: The list.
 *   &returns: The last shift.
 */

static struct shift_t *list_last(struct list_t *list)
{
	return &list->arr[list->len - 1];
}

/**
 * Find the number smaller than the value.
 *   @list: The list.
 *   @val: The value.
 *   &returns: The shift, if it exists.
 */

static struct shift_t *list_smaller(struct list_t *list, mpz_t val)
{
	int l, h, m, cmp;

	l = 0;
	h = list->len - 1;

	while(l <= h) {
		m = (l + h) / 2;
		cmp = mpz_cmpabs(list->arr[m].val, val);
		if(cmp > 0)
			l = m + 1;
		else if(cmp < 0)
			h = m - 1;
		else
			return (m < list->len-1) ? &list->arr[m+1] : NULL;
	}

	return (l < list->len) ? &list->arr[l] : NULL;
}

/**
 * Find the number at most the value.
 *   @list: The list.
 *   @val: The value.
 *   &returns: The shift, if it exists.
 */

static struct shift_t *list_atmost(struct list_t *list, mpz_t val)
{
	int l, h, m, cmp;

	l = 0;
	h = list->len - 1;

	while(l <= h) {
		m = (l + h) / 2;
		cmp = mpz_cmpabs(list->arr[m].val, val);
		if(cmp > 0)
			l = m + 1;
		else if(cmp < 0)
			h = m - 1;
		else
			return &list->arr[m+1];
	}

	return (l < list->len) ? &list->arr[l] : NULL;
}


/**
 * Create an index array.
 *   @len: The array length pointer.
 *   &returns: The empty array.
 */

static int64_t *arr_new(unsigned int *len)
{
	int64_t *arr;

	arr = malloc(sizeof(int64_t));
	arr[0] = -1;

	*len = 0;

	return arr;
}

/**
 * Add to index array.
 *   @arr: The array pointer.
 *   @len: the length pointer.
 *   @idx: The index to add.
 */

static void arr_add(int64_t **arr, unsigned int *len, int64_t idx)
{
	(*arr) = realloc(*arr, (*len + 2) * sizeof(int64_t));
	(*arr)[*len+0] = idx;
	(*arr)[*len+1] = -1;
	(*len)++;
}


/**
 * Initialize the exhaustive set.
 *   &returns: The set.
 */

static struct set_t set_init(void)
{
	return (struct set_t){ malloc(0), 0 };
}

/**
 * Destroy the exhaustive set.
 *   @set: The set.
 */

static void set_destroy(struct set_t *set)
{
	unsigned int i;

	for(i = 0; i < set->len; i++)
		mpz_clear(set->arr[i].val);
}

static void set_add(struct set_t *set, uint64_t idx, mpz_t val)
{
	unsigned int i;

	for(i = 0; i < set->len; i++) {
		if(set->arr[i].idx == idx)
			return;
	}

	set->arr = realloc(set->arr, (set->len + 1) * sizeof(struct point_t));
	set->arr[set->len].idx = idx;
	mpz_init(set->arr[set->len].val);
	mpz_set(set->arr[set->len].val, val);
	set->len++;
}
