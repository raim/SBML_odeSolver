/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include "unittest.h"

#include <sbmlsolver/ASTIndexNameNode.h>

/* test cases */
START_TEST(test_ASTNode_createIndexName)
{
	ASTNode_t *node;

	node = ASTNode_createIndexName();
	ck_assert(node != NULL);
	ck_assert(ASTNode_isIndexName(node));
	ASTNode_free(node);
}
END_TEST

START_TEST(test_ASTNode_setIndex)
{
	ASTNode_t *node;
	unsigned int i;

	node = ASTNode_createIndexName();
	ck_assert(node != NULL);
	i = ASTNode_getIndex(node);
	ck_assert(i == 0);
	ck_assert(!ASTNode_isSetIndex(node));
	ASTNode_setIndex(node, 100);
	i = ASTNode_getIndex(node);
	ck_assert(i == 100);
	ck_assert(ASTNode_isSetIndex(node));
	ASTNode_free(node);
}
END_TEST

START_TEST(test_ASTNode_setData)
{
	ASTNode_t *node;
	unsigned int i;

	node = ASTNode_createIndexName();
	ck_assert(node != NULL);
	i = ASTNode_isSetData(node);
	ck_assert(i == 0);
	ASTNode_setData(node);
	i = ASTNode_isSetData(node);
	ck_assert(i == 1);
	ASTNode_free(node);
}
END_TEST

/* public */
Suite *create_suite_ASTIndexNameNode(void)
{
	Suite *s;
	TCase *tc_ASTNode_createIndexName;
	TCase *tc_ASTNode_setIndex;
	TCase *tc_ASTNode_setData;

	s = suite_create("ASTIndexNameNode");

	tc_ASTNode_createIndexName = tcase_create("ASTNode_createIndexName");
	tcase_add_test(tc_ASTNode_createIndexName, test_ASTNode_createIndexName);
	suite_add_tcase(s, tc_ASTNode_createIndexName);

	tc_ASTNode_setIndex = tcase_create("ASTNode_setIndex");
	tcase_add_test(tc_ASTNode_setIndex, test_ASTNode_setIndex);
	suite_add_tcase(s, tc_ASTNode_setIndex);

	tc_ASTNode_setData = tcase_create("ASTNode_setData");
	tcase_add_test(tc_ASTNode_setData, test_ASTNode_setData);
	suite_add_tcase(s, tc_ASTNode_setData);

	return s;
}
