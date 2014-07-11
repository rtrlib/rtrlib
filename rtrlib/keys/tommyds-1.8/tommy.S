
tommy.o:     file format elf32-i386


Disassembly of section .text:

00000000 <tommy_hashdyn_resize>:

/**
 * Resize the bucket vector.
 */
static void tommy_hashdyn_resize(tommy_hashdyn* hashdyn, unsigned new_bucket_bit)
{
       0:	55                   	push   %ebp
       1:	57                   	push   %edi
	tommy_hashdyn_node** new_bucket;

	bucket_bit = hashdyn->bucket_bit;
	bucket_max = hashdyn->bucket_max;

	new_bucket_max = 1 << new_bucket_bit;
       2:	89 d1                	mov    %edx,%ecx

/**
 * Resize the bucket vector.
 */
static void tommy_hashdyn_resize(tommy_hashdyn* hashdyn, unsigned new_bucket_bit)
{
       4:	56                   	push   %esi
       5:	53                   	push   %ebx
       6:	83 ec 38             	sub    $0x38,%esp
	unsigned bucket_max;
	unsigned new_bucket_max;
	unsigned new_bucket_mask;
	tommy_hashdyn_node** new_bucket;

	bucket_bit = hashdyn->bucket_bit;
       9:	8b 78 04             	mov    0x4(%eax),%edi

/**
 * Resize the bucket vector.
 */
static void tommy_hashdyn_resize(tommy_hashdyn* hashdyn, unsigned new_bucket_bit)
{
       c:	89 44 24 1c          	mov    %eax,0x1c(%esp)
	unsigned new_bucket_max;
	unsigned new_bucket_mask;
	tommy_hashdyn_node** new_bucket;

	bucket_bit = hashdyn->bucket_bit;
	bucket_max = hashdyn->bucket_max;
      10:	8b 40 08             	mov    0x8(%eax),%eax

	new_bucket_max = 1 << new_bucket_bit;
      13:	89 54 24 28          	mov    %edx,0x28(%esp)
	unsigned new_bucket_max;
	unsigned new_bucket_mask;
	tommy_hashdyn_node** new_bucket;

	bucket_bit = hashdyn->bucket_bit;
	bucket_max = hashdyn->bucket_max;
      17:	89 44 24 20          	mov    %eax,0x20(%esp)

	new_bucket_max = 1 << new_bucket_bit;
      1b:	b8 01 00 00 00       	mov    $0x1,%eax
      20:	d3 e0                	shl    %cl,%eax
	new_bucket_mask = new_bucket_max - 1;
	new_bucket = tommy_cast(tommy_hashdyn_node**, tommy_malloc(new_bucket_max * sizeof(tommy_hashdyn_node*)));
      22:	8d 2c 85 00 00 00 00 	lea    0x0(,%eax,4),%ebp
	tommy_hashdyn_node** new_bucket;

	bucket_bit = hashdyn->bucket_bit;
	bucket_max = hashdyn->bucket_max;

	new_bucket_max = 1 << new_bucket_bit;
      29:	89 44 24 18          	mov    %eax,0x18(%esp)
	new_bucket_mask = new_bucket_max - 1;
      2d:	8d 70 ff             	lea    -0x1(%eax),%esi
	new_bucket = tommy_cast(tommy_hashdyn_node**, tommy_malloc(new_bucket_max * sizeof(tommy_hashdyn_node*)));
      30:	55                   	push   %ebp
      31:	e8 fc ff ff ff       	call   32 <tommy_hashdyn_resize+0x32>

	/* reinsert all the elements */
	if (new_bucket_bit > bucket_bit) {
      36:	83 c4 10             	add    $0x10,%esp
	bucket_bit = hashdyn->bucket_bit;
	bucket_max = hashdyn->bucket_max;

	new_bucket_max = 1 << new_bucket_bit;
	new_bucket_mask = new_bucket_max - 1;
	new_bucket = tommy_cast(tommy_hashdyn_node**, tommy_malloc(new_bucket_max * sizeof(tommy_hashdyn_node*)));
      39:	89 c3                	mov    %eax,%ebx

	/* reinsert all the elements */
	if (new_bucket_bit > bucket_bit) {
      3b:	3b 7c 24 1c          	cmp    0x1c(%esp),%edi
      3f:	0f 82 8f 00 00 00    	jb     d4 <tommy_hashdyn_resize+0xd4>
		}
	} else {
		unsigned i;

		/* shrink */
		for(i=0;i<new_bucket_max;++i) {
      45:	8b 44 24 0c          	mov    0xc(%esp),%eax
      49:	85 c0                	test   %eax,%eax
      4b:	0f 84 1b 01 00 00    	je     16c <tommy_hashdyn_resize+0x16c>
      51:	8b 44 24 10          	mov    0x10(%esp),%eax
      55:	89 74 24 18          	mov    %esi,0x18(%esp)
      59:	8b 38                	mov    (%eax),%edi
      5b:	8d 04 2f             	lea    (%edi,%ebp,1),%eax
      5e:	89 c5                	mov    %eax,%ebp
      60:	31 c0                	xor    %eax,%eax
      62:	89 6c 24 14          	mov    %ebp,0x14(%esp)
      66:	eb 1f                	jmp    87 <tommy_hashdyn_resize+0x87>
      68:	90                   	nop
      69:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
	tommy_node* head = tommy_list_head(list);

	if (!head)
		return 0;

	return head->prev;
      70:	8b 72 04             	mov    0x4(%edx),%esi
	first_head = tommy_list_head(first);
	second_head = tommy_list_head(second);
	first_tail = tommy_list_tail(first);

	/* set the "circular" prev list */
	first_head->prev = second_head->prev;
      73:	8b 69 04             	mov    0x4(%ecx),%ebp
      76:	89 6a 04             	mov    %ebp,0x4(%edx)
	second_head->prev = first_tail;
      79:	89 71 04             	mov    %esi,0x4(%ecx)

	/* set the "0 terminated" next list */
	first_tail->next = second_head;
      7c:	89 0e                	mov    %ecx,(%esi)
      7e:	83 c0 01             	add    $0x1,%eax
      81:	39 44 24 0c          	cmp    %eax,0xc(%esp)
      85:	74 21                	je     a8 <tommy_hashdyn_resize+0xa8>
			/* setup the new bucket with the lower bucket*/
			new_bucket[i] = hashdyn->bucket[i];
      87:	8b 14 87             	mov    (%edi,%eax,4),%edx
      8a:	8b 74 24 14          	mov    0x14(%esp),%esi
      8e:	89 14 83             	mov    %edx,(%ebx,%eax,4)
      91:	8b 0c 86             	mov    (%esi,%eax,4),%ecx
{
	tommy_node* first_head;
	tommy_node* first_tail;
	tommy_node* second_head;

	if (tommy_list_empty(second)) {
      94:	85 c9                	test   %ecx,%ecx
      96:	74 e6                	je     7e <tommy_hashdyn_resize+0x7e>
		return;
	}

	if (tommy_list_empty(first)) {
      98:	85 d2                	test   %edx,%edx
      9a:	75 d4                	jne    70 <tommy_hashdyn_resize+0x70>
		*first = *second;
      9c:	89 0c 83             	mov    %ecx,(%ebx,%eax,4)
		}
	} else {
		unsigned i;

		/* shrink */
		for(i=0;i<new_bucket_max;++i) {
      9f:	83 c0 01             	add    $0x1,%eax
      a2:	39 44 24 0c          	cmp    %eax,0xc(%esp)
      a6:	75 df                	jne    87 <tommy_hashdyn_resize+0x87>
      a8:	8b 74 24 18          	mov    0x18(%esp),%esi
			/* concat the upper bucket */
			tommy_list_concat(&new_bucket[i], &hashdyn->bucket[i + new_bucket_max]);
		}
	}

	tommy_free(hashdyn->bucket);
      ac:	83 ec 0c             	sub    $0xc,%esp
      af:	57                   	push   %edi
      b0:	e8 fc ff ff ff       	call   b1 <tommy_hashdyn_resize+0xb1>

	/* setup */
	hashdyn->bucket_bit = new_bucket_bit;
      b5:	8b 44 24 20          	mov    0x20(%esp),%eax
      b9:	8b 7c 24 2c          	mov    0x2c(%esp),%edi
      bd:	89 78 04             	mov    %edi,0x4(%eax)
	hashdyn->bucket_max = new_bucket_max;
      c0:	8b 7c 24 1c          	mov    0x1c(%esp),%edi
	hashdyn->bucket_mask = new_bucket_mask;
      c4:	89 70 0c             	mov    %esi,0xc(%eax)
	hashdyn->bucket = new_bucket;
      c7:	89 18                	mov    %ebx,(%eax)

	tommy_free(hashdyn->bucket);

	/* setup */
	hashdyn->bucket_bit = new_bucket_bit;
	hashdyn->bucket_max = new_bucket_max;
      c9:	89 78 08             	mov    %edi,0x8(%eax)
	hashdyn->bucket_mask = new_bucket_mask;
	hashdyn->bucket = new_bucket;
}
      cc:	83 c4 3c             	add    $0x3c,%esp
      cf:	5b                   	pop    %ebx
      d0:	5e                   	pop    %esi
      d1:	5f                   	pop    %edi
      d2:	5d                   	pop    %ebp
      d3:	c3                   	ret    
	/* reinsert all the elements */
	if (new_bucket_bit > bucket_bit) {
		unsigned i;

		/* grow */
		for(i=0;i<bucket_max;++i) {
      d4:	8b 44 24 14          	mov    0x14(%esp),%eax
      d8:	85 c0                	test   %eax,%eax
      da:	0f 84 8c 00 00 00    	je     16c <tommy_hashdyn_resize+0x16c>
      e0:	8b 7c 24 10          	mov    0x10(%esp),%edi
      e4:	c1 e0 02             	shl    $0x2,%eax
      e7:	31 ed                	xor    %ebp,%ebp
      e9:	89 44 24 18          	mov    %eax,0x18(%esp)
      ed:	01 d8                	add    %ebx,%eax
      ef:	89 44 24 14          	mov    %eax,0x14(%esp)
      f3:	8b 3f                	mov    (%edi),%edi
      f5:	8d 76 00             	lea    0x0(%esi),%esi
			tommy_hashdyn_node* j;

			/* setup the new two buckets */
			new_bucket[i] = 0;
			new_bucket[i + bucket_max] = 0;
      f8:	8b 44 24 14          	mov    0x14(%esp),%eax
		/* grow */
		for(i=0;i<bucket_max;++i) {
			tommy_hashdyn_node* j;

			/* setup the new two buckets */
			new_bucket[i] = 0;
      fc:	c7 04 2b 00 00 00 00 	movl   $0x0,(%ebx,%ebp,1)
			new_bucket[i + bucket_max] = 0;
     103:	c7 04 28 00 00 00 00 	movl   $0x0,(%eax,%ebp,1)

			/* reinsert the bucket */
			j = hashdyn->bucket[i];
     10a:	8b 04 2f             	mov    (%edi,%ebp,1),%eax
			while (j) {
     10d:	85 c0                	test   %eax,%eax
     10f:	75 21                	jne    132 <tommy_hashdyn_resize+0x132>
     111:	eb 4b                	jmp    15e <tommy_hashdyn_resize+0x15e>
     113:	90                   	nop
     114:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_tail_not_empty(tommy_node* head, tommy_node* node)
{
	/* insert in the "circular" prev list */
	node->prev = head->prev;
     118:	8b 4a 04             	mov    0x4(%edx),%ecx
     11b:	85 ff                	test   %edi,%edi
     11d:	89 48 04             	mov    %ecx,0x4(%eax)
	head->prev = node;
     120:	89 42 04             	mov    %eax,0x4(%edx)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
     123:	8b 50 04             	mov    0x4(%eax),%edx
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
     126:	c7 00 00 00 00 00    	movl   $0x0,(%eax)
	node->prev->next = node;
     12c:	89 02                	mov    %eax,(%edx)
     12e:	89 f8                	mov    %edi,%eax
     130:	74 26                	je     158 <tommy_hashdyn_resize+0x158>
				tommy_hashdyn_node* j_next = j->next;
				unsigned index = j->key & new_bucket_mask;
     132:	8b 50 0c             	mov    0xc(%eax),%edx
			new_bucket[i + bucket_max] = 0;

			/* reinsert the bucket */
			j = hashdyn->bucket[i];
			while (j) {
				tommy_hashdyn_node* j_next = j->next;
     135:	8b 38                	mov    (%eax),%edi
				unsigned index = j->key & new_bucket_mask;
     137:	21 f2                	and    %esi,%edx
				if (new_bucket[index])
     139:	8d 0c 93             	lea    (%ebx,%edx,4),%ecx
     13c:	8b 11                	mov    (%ecx),%edx
     13e:	85 d2                	test   %edx,%edx
     140:	75 d6                	jne    118 <tommy_hashdyn_resize+0x118>
			new_bucket[i] = 0;
			new_bucket[i + bucket_max] = 0;

			/* reinsert the bucket */
			j = hashdyn->bucket[i];
			while (j) {
     142:	85 ff                	test   %edi,%edi
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
     144:	89 40 04             	mov    %eax,0x4(%eax)

	/* one element "0 terminated" next list */
	node->next = 0;
     147:	c7 00 00 00 00 00    	movl   $0x0,(%eax)

	*list = node;
     14d:	89 01                	mov    %eax,(%ecx)
     14f:	89 f8                	mov    %edi,%eax
     151:	75 df                	jne    132 <tommy_hashdyn_resize+0x132>
     153:	90                   	nop
     154:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
     158:	8b 44 24 10          	mov    0x10(%esp),%eax
     15c:	8b 38                	mov    (%eax),%edi
     15e:	83 c5 04             	add    $0x4,%ebp
	/* reinsert all the elements */
	if (new_bucket_bit > bucket_bit) {
		unsigned i;

		/* grow */
		for(i=0;i<bucket_max;++i) {
     161:	3b 6c 24 18          	cmp    0x18(%esp),%ebp
     165:	75 91                	jne    f8 <tommy_hashdyn_resize+0xf8>
     167:	e9 40 ff ff ff       	jmp    ac <tommy_hashdyn_resize+0xac>
     16c:	8b 44 24 10          	mov    0x10(%esp),%eax
     170:	8b 38                	mov    (%eax),%edi
     172:	e9 35 ff ff ff       	jmp    ac <tommy_hashdyn_resize+0xac>
     177:	89 f6                	mov    %esi,%esi
     179:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000180 <tommy_chain_mergesort>:
 *
 * Linux Kernel lib/list_sort.c
 * http://lxr.linux.no/#linux+v2.6.36/lib/list_sort.c
 */
tommy_inline void tommy_chain_mergesort(tommy_chain* chain, tommy_compare_func* cmp)
{
     180:	55                   	push   %ebp
     181:	57                   	push   %edi
     182:	89 d5                	mov    %edx,%ebp
     184:	56                   	push   %esi
     185:	53                   	push   %ebx
     186:	81 ec 4c 01 00 00    	sub    $0x14c,%esp
	/**
	 * Value stored inside the bit bucket.
	 * It's used to know which bucket is empty of full.
	 */
	unsigned counter;
	tommy_node* node = chain->head;
     18c:	8b 08                	mov    (%eax),%ecx
 *
 * Linux Kernel lib/list_sort.c
 * http://lxr.linux.no/#linux+v2.6.36/lib/list_sort.c
 */
tommy_inline void tommy_chain_mergesort(tommy_chain* chain, tommy_compare_func* cmp)
{
     18e:	89 44 24 2c          	mov    %eax,0x2c(%esp)
	 * Value stored inside the bit bucket.
	 * It's used to know which bucket is empty of full.
	 */
	unsigned counter;
	tommy_node* node = chain->head;
	tommy_node* tail = chain->tail;
     192:	8b 40 04             	mov    0x4(%eax),%eax
	unsigned mask;
	unsigned i;

	counter = 0;
     195:	c7 44 24 20 00 00 00 	movl   $0x0,0x20(%esp)
     19c:	00 
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
		next = node->next;

		/* add the bit, propagating the carry */
		i = 0;
     19d:	c7 44 24 18 00 00 00 	movl   $0x0,0x18(%esp)
     1a4:	00 
		tommy_node* next;
		tommy_chain* last;

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
     1a5:	89 8c 24 38 01 00 00 	mov    %ecx,0x138(%esp)
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
     1ac:	89 8c 24 3c 01 00 00 	mov    %ecx,0x13c(%esp)
	 * Value stored inside the bit bucket.
	 * It's used to know which bucket is empty of full.
	 */
	unsigned counter;
	tommy_node* node = chain->head;
	tommy_node* tail = chain->tail;
     1b3:	89 44 24 28          	mov    %eax,0x28(%esp)

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
		next = node->next;
     1b7:	8b 01                	mov    (%ecx),%eax
     1b9:	89 44 24 1c          	mov    %eax,0x1c(%esp)
	while (1) {
		tommy_node* next;
		tommy_chain* last;

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
     1bd:	8d 84 24 38 01 00 00 	lea    0x138(%esp),%eax
     1c4:	89 44 24 14          	mov    %eax,0x14(%esp)
     1c8:	90                   	nop
     1c9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

		/* copy the carry in the first empty bit */
		bit[i] = *last;

		/* add the carry in the counter */
		++counter;
     1d0:	83 44 24 20 01       	addl   $0x1,0x20(%esp)

		if (node == tail)
     1d5:	3b 4c 24 28          	cmp    0x28(%esp),%ecx
			last = &bit[i];
			++i;
		}

		/* copy the carry in the first empty bit */
		bit[i] = *last;
     1d9:	8b 50 04             	mov    0x4(%eax),%edx
     1dc:	8b 5c 24 18          	mov    0x18(%esp),%ebx
     1e0:	8b 00                	mov    (%eax),%eax
     1e2:	89 54 dc 3c          	mov    %edx,0x3c(%esp,%ebx,8)
     1e6:	89 44 dc 38          	mov    %eax,0x38(%esp,%ebx,8)

		/* add the carry in the counter */
		++counter;

		if (node == tail)
     1ea:	0f 84 6d 01 00 00    	je     35d <tommy_chain_mergesort+0x1dd>
		tommy_node* next;
		tommy_chain* last;

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
     1f0:	8b 44 24 1c          	mov    0x1c(%esp),%eax
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
		next = node->next;
     1f4:	8b 10                	mov    (%eax),%edx
		tommy_node* next;
		tommy_chain* last;

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
     1f6:	89 84 24 38 01 00 00 	mov    %eax,0x138(%esp)
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
     1fd:	89 84 24 3c 01 00 00 	mov    %eax,0x13c(%esp)
		next = node->next;
     204:	89 54 24 24          	mov    %edx,0x24(%esp)

		/* add the bit, propagating the carry */
		i = 0;
		mask = counter;
		while ((mask & 1) != 0) {
     208:	8b 54 24 20          	mov    0x20(%esp),%edx
     20c:	f6 c2 01             	test   $0x1,%dl
     20f:	0f 84 1c 01 00 00    	je     331 <tommy_chain_mergesort+0x1b1>
     215:	89 44 24 10          	mov    %eax,0x10(%esp)
     219:	89 c7                	mov    %eax,%edi
     21b:	8d 84 24 38 01 00 00 	lea    0x138(%esp),%eax
     222:	8d 5c 24 38          	lea    0x38(%esp),%ebx
     226:	89 54 24 0c          	mov    %edx,0xc(%esp)
     22a:	c7 44 24 18 00 00 00 	movl   $0x0,0x18(%esp)
     231:	00 
     232:	89 44 24 08          	mov    %eax,0x8(%esp)
     236:	8d 76 00             	lea    0x0(%esi),%esi
     239:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
 * It's funtionally equivalent at tommy_chain_merge() but faster with already ordered chains.
 */
tommy_inline void tommy_chain_merge_degenerated(tommy_chain* first, tommy_chain* second, tommy_compare_func* cmp)
{
	/* identify the condition first <= second */
	if (cmp(first->tail->data, second->head->data) <= 0) {
     240:	8b 73 04             	mov    0x4(%ebx),%esi

		/* add the bit, propagating the carry */
		i = 0;
		mask = counter;
		while ((mask & 1) != 0) {
			tommy_chain_merge_degenerated(&bit[i], last, cmp);
     243:	89 5c 24 14          	mov    %ebx,0x14(%esp)
 * It's funtionally equivalent at tommy_chain_merge() but faster with already ordered chains.
 */
tommy_inline void tommy_chain_merge_degenerated(tommy_chain* first, tommy_chain* second, tommy_compare_func* cmp)
{
	/* identify the condition first <= second */
	if (cmp(first->tail->data, second->head->data) <= 0) {
     247:	83 ec 08             	sub    $0x8,%esp
     24a:	ff 77 08             	pushl  0x8(%edi)
     24d:	ff 76 08             	pushl  0x8(%esi)
     250:	ff d5                	call   *%ebp
     252:	83 c4 10             	add    $0x10,%esp
     255:	85 c0                	test   %eax,%eax
     257:	7e 5f                	jle    2b8 <tommy_chain_mergesort+0x138>
		return;
	}

	/* identify the condition second < first */
	/* here we must be strict on comparison to keep the sort stable */
	if (cmp(second->tail->data, first->head->data) < 0) {
     259:	8b 33                	mov    (%ebx),%esi
     25b:	83 ec 08             	sub    $0x8,%esp
     25e:	ff 76 08             	pushl  0x8(%esi)
     261:	8b 44 24 1c          	mov    0x1c(%esp),%eax
     265:	ff 70 08             	pushl  0x8(%eax)
     268:	ff d5                	call   *%ebp
     26a:	83 c4 10             	add    $0x10,%esp
     26d:	85 c0                	test   %eax,%eax
     26f:	0f 88 9b 00 00 00    	js     310 <tommy_chain_mergesort+0x190>
 * Merges two chains.
 */
tommy_inline void tommy_chain_merge(tommy_chain* first, tommy_chain* second, tommy_compare_func* cmp)
{
	tommy_node* first_i = first->head;
	tommy_node* second_i = second->head;
     275:	8b 44 24 08          	mov    0x8(%esp),%eax
     279:	8b 38                	mov    (%eax),%edi
     27b:	90                   	nop
     27c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

	/* merge */
	while (1) {
		if (cmp(first_i->data, second_i->data) > 0) {
     280:	83 ec 08             	sub    $0x8,%esp
     283:	ff 77 08             	pushl  0x8(%edi)
     286:	ff 76 08             	pushl  0x8(%esi)
     289:	ff d5                	call   *%ebp
     28b:	83 c4 10             	add    $0x10,%esp
     28e:	85 c0                	test   %eax,%eax
     290:	7e 5e                	jle    2f0 <tommy_chain_mergesort+0x170>
			tommy_node* next = second_i->next;
			if (first_i == first->head) {
     292:	3b 33                	cmp    (%ebx),%esi
	tommy_node* second_i = second->head;

	/* merge */
	while (1) {
		if (cmp(first_i->data, second_i->data) > 0) {
			tommy_node* next = second_i->next;
     294:	8b 07                	mov    (%edi),%eax
			if (first_i == first->head) {
     296:	74 68                	je     300 <tommy_chain_mergesort+0x180>
				tommy_chain_concat(second_i, first_i);
				first->head = second_i;
			} else {
				tommy_chain_splice(first_i->prev, first_i, second_i, second_i);
     298:	8b 4e 04             	mov    0x4(%esi),%ecx
 * Splices a chain in the middle of another chain.
 */
tommy_inline void tommy_chain_splice(tommy_node* first_before, tommy_node* first_after, tommy_node* second_head, tommy_node* second_tail)
{
	/* set the prev list */
	first_after->prev = second_tail;
     29b:	89 7e 04             	mov    %edi,0x4(%esi)
	second_head->prev = first_before;
     29e:	89 4f 04             	mov    %ecx,0x4(%edi)

	/* set the next list */
	first_before->next = second_head;
     2a1:	89 39                	mov    %edi,(%ecx)
	second_tail->next = first_after;
     2a3:	89 37                	mov    %esi,(%edi)
				tommy_chain_concat(second_i, first_i);
				first->head = second_i;
			} else {
				tommy_chain_splice(first_i->prev, first_i, second_i, second_i);
			}
			if (second_i == second->tail)
     2a5:	8b 54 24 08          	mov    0x8(%esp),%edx
     2a9:	3b 7a 04             	cmp    0x4(%edx),%edi
     2ac:	74 16                	je     2c4 <tommy_chain_mergesort+0x144>
			if (first_i == first->tail) {
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
				break;
			}
			first_i = first_i->next;
     2ae:	89 c7                	mov    %eax,%edi
     2b0:	eb ce                	jmp    280 <tommy_chain_mergesort+0x100>
     2b2:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
tommy_inline void tommy_chain_merge_degenerated(tommy_chain* first, tommy_chain* second, tommy_compare_func* cmp)
{
	/* identify the condition first <= second */
	if (cmp(first->tail->data, second->head->data) <= 0) {
		tommy_chain_concat(first->tail, second->head);
		first->tail = second->tail;
     2b8:	8b 44 24 10          	mov    0x10(%esp),%eax
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     2bc:	89 77 04             	mov    %esi,0x4(%edi)

	/* set the next list */
	first_tail->next = second_head;
     2bf:	89 3e                	mov    %edi,(%esi)
tommy_inline void tommy_chain_merge_degenerated(tommy_chain* first, tommy_chain* second, tommy_compare_func* cmp)
{
	/* identify the condition first <= second */
	if (cmp(first->tail->data, second->head->data) <= 0) {
		tommy_chain_concat(first->tail, second->head);
		first->tail = second->tail;
     2c1:	89 43 04             	mov    %eax,0x4(%ebx)
		/* add the bit, propagating the carry */
		i = 0;
		mask = counter;
		while ((mask & 1) != 0) {
			tommy_chain_merge_degenerated(&bit[i], last, cmp);
			mask >>= 1;
     2c4:	d1 6c 24 0c          	shrl   0xc(%esp)
			last = &bit[i];
			++i;
     2c8:	83 44 24 18 01       	addl   $0x1,0x18(%esp)
		/* add the bit, propagating the carry */
		i = 0;
		mask = counter;
		while ((mask & 1) != 0) {
			tommy_chain_merge_degenerated(&bit[i], last, cmp);
			mask >>= 1;
     2cd:	8b 44 24 0c          	mov    0xc(%esp),%eax
		next = node->next;

		/* add the bit, propagating the carry */
		i = 0;
		mask = counter;
		while ((mask & 1) != 0) {
     2d1:	a8 01                	test   $0x1,%al
     2d3:	74 73                	je     348 <tommy_chain_mergesort+0x1c8>
     2d5:	8b 43 04             	mov    0x4(%ebx),%eax
     2d8:	8b 3b                	mov    (%ebx),%edi
     2da:	83 c3 08             	add    $0x8,%ebx
     2dd:	89 44 24 10          	mov    %eax,0x10(%esp)
     2e1:	8b 44 24 14          	mov    0x14(%esp),%eax
     2e5:	89 44 24 08          	mov    %eax,0x8(%esp)
     2e9:	e9 52 ff ff ff       	jmp    240 <tommy_chain_mergesort+0xc0>
     2ee:	66 90                	xchg   %ax,%ax
			}
			if (second_i == second->tail)
				break;
			second_i = next;
		} else {
			if (first_i == first->tail) {
     2f0:	3b 73 04             	cmp    0x4(%ebx),%esi
     2f3:	74 2b                	je     320 <tommy_chain_mergesort+0x1a0>
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
				break;
			}
			first_i = first_i->next;
     2f5:	89 f8                	mov    %edi,%eax
     2f7:	8b 36                	mov    (%esi),%esi
     2f9:	89 c7                	mov    %eax,%edi
     2fb:	eb 83                	jmp    280 <tommy_chain_mergesort+0x100>
     2fd:	8d 76 00             	lea    0x0(%esi),%esi
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     300:	89 7e 04             	mov    %edi,0x4(%esi)
	while (1) {
		if (cmp(first_i->data, second_i->data) > 0) {
			tommy_node* next = second_i->next;
			if (first_i == first->head) {
				tommy_chain_concat(second_i, first_i);
				first->head = second_i;
     303:	89 3b                	mov    %edi,(%ebx)
{
	/* set the prev list */
	second_head->prev = first_tail;

	/* set the next list */
	first_tail->next = second_head;
     305:	89 37                	mov    %esi,(%edi)
     307:	eb 9c                	jmp    2a5 <tommy_chain_mergesort+0x125>
     309:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     310:	8b 44 24 10          	mov    0x10(%esp),%eax

	/* identify the condition second < first */
	/* here we must be strict on comparison to keep the sort stable */
	if (cmp(second->tail->data, first->head->data) < 0) {
		tommy_chain_concat(second->tail, first->head);
		first->head = second->head;
     314:	89 3b                	mov    %edi,(%ebx)
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     316:	89 46 04             	mov    %eax,0x4(%esi)

	/* set the next list */
	first_tail->next = second_head;
     319:	89 30                	mov    %esi,(%eax)
     31b:	eb a7                	jmp    2c4 <tommy_chain_mergesort+0x144>
     31d:	8d 76 00             	lea    0x0(%esi),%esi
				break;
			second_i = next;
		} else {
			if (first_i == first->tail) {
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
     320:	8b 44 24 08          	mov    0x8(%esp),%eax
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     324:	89 77 04             	mov    %esi,0x4(%edi)

	/* set the next list */
	first_tail->next = second_head;
     327:	89 3e                	mov    %edi,(%esi)
				break;
			second_i = next;
		} else {
			if (first_i == first->tail) {
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
     329:	8b 40 04             	mov    0x4(%eax),%eax
     32c:	89 43 04             	mov    %eax,0x4(%ebx)
     32f:	eb 93                	jmp    2c4 <tommy_chain_mergesort+0x144>
	while (1) {
		tommy_node* next;
		tommy_chain* last;

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
     331:	8d 84 24 38 01 00 00 	lea    0x138(%esp),%eax
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
		next = node->next;

		/* add the bit, propagating the carry */
		i = 0;
     338:	c7 44 24 18 00 00 00 	movl   $0x0,0x18(%esp)
     33f:	00 
	while (1) {
		tommy_node* next;
		tommy_chain* last;

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
     340:	89 44 24 14          	mov    %eax,0x14(%esp)
     344:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
		next = node->next;
     348:	8b 44 24 24          	mov    0x24(%esp),%eax

		/* add the bit, propagating the carry */
		i = 0;
     34c:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx

		/* carry bit to add */
		last = &bit[TOMMY_CHAIN_BIT_MAX];
		bit[TOMMY_CHAIN_BIT_MAX].head = node;
		bit[TOMMY_CHAIN_BIT_MAX].tail = node;
		next = node->next;
     350:	89 44 24 1c          	mov    %eax,0x1c(%esp)
     354:	8b 44 24 14          	mov    0x14(%esp),%eax
     358:	e9 73 fe ff ff       	jmp    1d0 <tommy_chain_mergesort+0x50>
#if defined(_MSC_VER)
	unsigned long count;
	_BitScanForward(&count, value);
	return count;
#elif defined(__GNUC__)
	return __builtin_ctz(value);
     35d:	8b 7c 24 20          	mov    0x20(%esp),%edi
     361:	f3 0f bc c7          	tzcnt  %edi,%eax
		node = next;
	}

	/* merge the buckets */
	i = tommy_ctz_u32(counter);
	mask = counter >> i;
     365:	89 c1                	mov    %eax,%ecx
     367:	89 44 24 08          	mov    %eax,0x8(%esp)
     36b:	d3 ef                	shr    %cl,%edi
	while (mask != 1) {
     36d:	83 ff 01             	cmp    $0x1,%edi
     370:	0f 84 e0 00 00 00    	je     456 <tommy_chain_mergesort+0x2d6>
     376:	8d 4c c4 38          	lea    0x38(%esp,%eax,8),%ecx
     37a:	83 c0 01             	add    $0x1,%eax
     37d:	89 6c 24 0c          	mov    %ebp,0xc(%esp)
     381:	89 44 24 08          	mov    %eax,0x8(%esp)
     385:	89 cd                	mov    %ecx,%ebp
     387:	eb 23                	jmp    3ac <tommy_chain_mergesort+0x22c>
		mask >>= 1;
		if (mask & 1) {
			tommy_chain_merge_degenerated(&bit[i+1], &bit[i], cmp);
		} else {
			bit[i+1] = bit[i];
     389:	8b 45 00             	mov    0x0(%ebp),%eax
     38c:	8b 55 04             	mov    0x4(%ebp),%edx
     38f:	89 45 08             	mov    %eax,0x8(%ebp)
     392:	89 55 0c             	mov    %edx,0xc(%ebp)
     395:	8b 44 24 08          	mov    0x8(%esp),%eax
     399:	83 c5 08             	add    $0x8,%ebp
     39c:	83 c0 01             	add    $0x1,%eax
	}

	/* merge the buckets */
	i = tommy_ctz_u32(counter);
	mask = counter >> i;
	while (mask != 1) {
     39f:	83 ff 01             	cmp    $0x1,%edi
     3a2:	0f 84 ae 00 00 00    	je     456 <tommy_chain_mergesort+0x2d6>
     3a8:	89 44 24 08          	mov    %eax,0x8(%esp)
		mask >>= 1;
     3ac:	d1 ef                	shr    %edi
		if (mask & 1) {
     3ae:	f7 c7 01 00 00 00    	test   $0x1,%edi
     3b4:	74 d3                	je     389 <tommy_chain_mergesort+0x209>
 * It's funtionally equivalent at tommy_chain_merge() but faster with already ordered chains.
 */
tommy_inline void tommy_chain_merge_degenerated(tommy_chain* first, tommy_chain* second, tommy_compare_func* cmp)
{
	/* identify the condition first <= second */
	if (cmp(first->tail->data, second->head->data) <= 0) {
     3b6:	8b 5d 00             	mov    0x0(%ebp),%ebx
     3b9:	8b 75 0c             	mov    0xc(%ebp),%esi
     3bc:	83 ec 08             	sub    $0x8,%esp
     3bf:	ff 73 08             	pushl  0x8(%ebx)
     3c2:	ff 76 08             	pushl  0x8(%esi)
     3c5:	8b 44 24 1c          	mov    0x1c(%esp),%eax
     3c9:	ff d0                	call   *%eax
     3cb:	83 c4 10             	add    $0x10,%esp
     3ce:	85 c0                	test   %eax,%eax
     3d0:	0f 8e bd 00 00 00    	jle    493 <tommy_chain_mergesort+0x313>
		return;
	}

	/* identify the condition second < first */
	/* here we must be strict on comparison to keep the sort stable */
	if (cmp(second->tail->data, first->head->data) < 0) {
     3d6:	8b 55 04             	mov    0x4(%ebp),%edx
     3d9:	8b 75 08             	mov    0x8(%ebp),%esi
     3dc:	83 ec 08             	sub    $0x8,%esp
     3df:	ff 76 08             	pushl  0x8(%esi)
     3e2:	ff 72 08             	pushl  0x8(%edx)
     3e5:	8b 44 24 1c          	mov    0x1c(%esp),%eax
     3e9:	89 54 24 20          	mov    %edx,0x20(%esp)
     3ed:	ff d0                	call   *%eax
     3ef:	83 c4 10             	add    $0x10,%esp
     3f2:	85 c0                	test   %eax,%eax
     3f4:	8b 54 24 10          	mov    0x10(%esp),%edx
     3f8:	0f 88 a5 00 00 00    	js     4a3 <tommy_chain_mergesort+0x323>
     3fe:	89 7c 24 10          	mov    %edi,0x10(%esp)
     402:	89 f7                	mov    %esi,%edi
     404:	8b 74 24 0c          	mov    0xc(%esp),%esi
     408:	90                   	nop
     409:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
	tommy_node* first_i = first->head;
	tommy_node* second_i = second->head;

	/* merge */
	while (1) {
		if (cmp(first_i->data, second_i->data) > 0) {
     410:	83 ec 08             	sub    $0x8,%esp
     413:	ff 73 08             	pushl  0x8(%ebx)
     416:	ff 77 08             	pushl  0x8(%edi)
     419:	ff d6                	call   *%esi
     41b:	83 c4 10             	add    $0x10,%esp
     41e:	85 c0                	test   %eax,%eax
     420:	7e 54                	jle    476 <tommy_chain_mergesort+0x2f6>
			tommy_node* next = second_i->next;
			if (first_i == first->head) {
     422:	3b 7d 08             	cmp    0x8(%ebp),%edi
	tommy_node* second_i = second->head;

	/* merge */
	while (1) {
		if (cmp(first_i->data, second_i->data) > 0) {
			tommy_node* next = second_i->next;
     425:	8b 03                	mov    (%ebx),%eax
			if (first_i == first->head) {
     427:	74 5a                	je     483 <tommy_chain_mergesort+0x303>
				tommy_chain_concat(second_i, first_i);
				first->head = second_i;
			} else {
				tommy_chain_splice(first_i->prev, first_i, second_i, second_i);
     429:	8b 4f 04             	mov    0x4(%edi),%ecx
 * Splices a chain in the middle of another chain.
 */
tommy_inline void tommy_chain_splice(tommy_node* first_before, tommy_node* first_after, tommy_node* second_head, tommy_node* second_tail)
{
	/* set the prev list */
	first_after->prev = second_tail;
     42c:	89 5f 04             	mov    %ebx,0x4(%edi)
	second_head->prev = first_before;
     42f:	89 4b 04             	mov    %ecx,0x4(%ebx)

	/* set the next list */
	first_before->next = second_head;
     432:	89 19                	mov    %ebx,(%ecx)
	second_tail->next = first_after;
     434:	89 3b                	mov    %edi,(%ebx)
				tommy_chain_concat(second_i, first_i);
				first->head = second_i;
			} else {
				tommy_chain_splice(first_i->prev, first_i, second_i, second_i);
			}
			if (second_i == second->tail)
     436:	3b 5d 04             	cmp    0x4(%ebp),%ebx
     439:	74 04                	je     43f <tommy_chain_mergesort+0x2bf>
			if (first_i == first->tail) {
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
				break;
			}
			first_i = first_i->next;
     43b:	89 c3                	mov    %eax,%ebx
     43d:	eb d1                	jmp    410 <tommy_chain_mergesort+0x290>
     43f:	8b 7c 24 10          	mov    0x10(%esp),%edi
     443:	8b 44 24 08          	mov    0x8(%esp),%eax
     447:	83 c5 08             	add    $0x8,%ebp
     44a:	83 c0 01             	add    $0x1,%eax
	}

	/* merge the buckets */
	i = tommy_ctz_u32(counter);
	mask = counter >> i;
	while (mask != 1) {
     44d:	83 ff 01             	cmp    $0x1,%edi
     450:	0f 85 52 ff ff ff    	jne    3a8 <tommy_chain_mergesort+0x228>
			bit[i+1] = bit[i];
		}
		++i;
	}

	*chain = bit[i];
     456:	8b 44 24 08          	mov    0x8(%esp),%eax
     45a:	8b 4c 24 2c          	mov    0x2c(%esp),%ecx
     45e:	8b 54 c4 3c          	mov    0x3c(%esp,%eax,8),%edx
     462:	8b 44 c4 38          	mov    0x38(%esp,%eax,8),%eax
     466:	89 51 04             	mov    %edx,0x4(%ecx)
     469:	89 01                	mov    %eax,(%ecx)
}
     46b:	81 c4 4c 01 00 00    	add    $0x14c,%esp
     471:	5b                   	pop    %ebx
     472:	5e                   	pop    %esi
     473:	5f                   	pop    %edi
     474:	5d                   	pop    %ebp
     475:	c3                   	ret    
			}
			if (second_i == second->tail)
				break;
			second_i = next;
		} else {
			if (first_i == first->tail) {
     476:	3b 7d 0c             	cmp    0xc(%ebp),%edi
     479:	74 12                	je     48d <tommy_chain_mergesort+0x30d>
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
				break;
			}
			first_i = first_i->next;
     47b:	89 d8                	mov    %ebx,%eax
     47d:	8b 3f                	mov    (%edi),%edi
     47f:	89 c3                	mov    %eax,%ebx
     481:	eb 8d                	jmp    410 <tommy_chain_mergesort+0x290>
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     483:	89 5f 04             	mov    %ebx,0x4(%edi)
	while (1) {
		if (cmp(first_i->data, second_i->data) > 0) {
			tommy_node* next = second_i->next;
			if (first_i == first->head) {
				tommy_chain_concat(second_i, first_i);
				first->head = second_i;
     486:	89 5d 08             	mov    %ebx,0x8(%ebp)
{
	/* set the prev list */
	second_head->prev = first_tail;

	/* set the next list */
	first_tail->next = second_head;
     489:	89 3b                	mov    %edi,(%ebx)
     48b:	eb a9                	jmp    436 <tommy_chain_mergesort+0x2b6>
     48d:	89 fe                	mov    %edi,%esi
     48f:	8b 7c 24 10          	mov    0x10(%esp),%edi
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     493:	89 73 04             	mov    %esi,0x4(%ebx)

	/* set the next list */
	first_tail->next = second_head;
     496:	89 1e                	mov    %ebx,(%esi)
				break;
			second_i = next;
		} else {
			if (first_i == first->tail) {
				tommy_chain_concat(first_i, second_i);
				first->tail = second->tail;
     498:	8b 45 04             	mov    0x4(%ebp),%eax
     49b:	89 45 0c             	mov    %eax,0xc(%ebp)
     49e:	e9 f2 fe ff ff       	jmp    395 <tommy_chain_mergesort+0x215>
 * Concats two chains.
 */
tommy_inline void tommy_chain_concat(tommy_node* first_tail, tommy_node* second_head)
{
	/* set the prev list */
	second_head->prev = first_tail;
     4a3:	89 56 04             	mov    %edx,0x4(%esi)

	/* identify the condition second < first */
	/* here we must be strict on comparison to keep the sort stable */
	if (cmp(second->tail->data, first->head->data) < 0) {
		tommy_chain_concat(second->tail, first->head);
		first->head = second->head;
     4a6:	89 5d 08             	mov    %ebx,0x8(%ebp)
{
	/* set the prev list */
	second_head->prev = first_tail;

	/* set the next list */
	first_tail->next = second_head;
     4a9:	89 32                	mov    %esi,(%edx)
     4ab:	e9 e5 fe ff ff       	jmp    395 <tommy_chain_mergesort+0x215>

000004b0 <tommy_hash_u32>:
		b ^= a; b -= tommy_rot(a,14); \
		c ^= b; c -= tommy_rot(b,24); \
	} while (0)

tommy_uint32_t tommy_hash_u32(tommy_uint32_t init_val, const void* void_key, tommy_size_t key_len)
{
     4b0:	57                   	push   %edi
     4b1:	56                   	push   %esi
     4b2:	53                   	push   %ebx
     4b3:	8b 4c 24 18          	mov    0x18(%esp),%ecx
	const unsigned char* key = tommy_cast(const unsigned char*,void_key);
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + init_val;
     4b7:	8b 44 24 10          	mov    0x10(%esp),%eax
		b ^= a; b -= tommy_rot(a,14); \
		c ^= b; c -= tommy_rot(b,24); \
	} while (0)

tommy_uint32_t tommy_hash_u32(tommy_uint32_t init_val, const void* void_key, tommy_size_t key_len)
{
     4bb:	8b 54 24 14          	mov    0x14(%esp),%edx
	const unsigned char* key = tommy_cast(const unsigned char*,void_key);
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + init_val;

	while (key_len > 12) {
     4bf:	83 f9 0c             	cmp    $0xc,%ecx
tommy_uint32_t tommy_hash_u32(tommy_uint32_t init_val, const void* void_key, tommy_size_t key_len)
{
	const unsigned char* key = tommy_cast(const unsigned char*,void_key);
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + init_val;
     4c2:	8d b4 08 ef be ad de 	lea    -0x21524111(%eax,%ecx,1),%esi

	while (key_len > 12) {
     4c9:	89 f0                	mov    %esi,%eax
     4cb:	89 f3                	mov    %esi,%ebx
     4cd:	76 57                	jbe    526 <tommy_hash_u32+0x76>
     4cf:	90                   	nop
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
		c += tommy_le_uint32_read(key + 8);
     4d0:	03 42 08             	add    0x8(%edx),%eax

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + init_val;

	while (key_len > 12) {
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
     4d3:	03 5a 04             	add    0x4(%edx),%ebx
		c += tommy_le_uint32_read(key + 8);

		tommy_mix(a,b,c);

		key_len -= 12;
     4d6:	83 e9 0c             	sub    $0xc,%ecx
		key += 12;
     4d9:	83 c2 0c             	add    $0xc,%edx
     4dc:	29 c6                	sub    %eax,%esi
	while (key_len > 12) {
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
		c += tommy_le_uint32_read(key + 8);

		tommy_mix(a,b,c);
     4de:	03 72 f4             	add    -0xc(%edx),%esi
     4e1:	89 c7                	mov    %eax,%edi
     4e3:	c1 c7 04             	rol    $0x4,%edi
     4e6:	01 d8                	add    %ebx,%eax
     4e8:	31 fe                	xor    %edi,%esi
     4ea:	89 f7                	mov    %esi,%edi
     4ec:	29 f3                	sub    %esi,%ebx
     4ee:	01 c6                	add    %eax,%esi
     4f0:	c1 c7 06             	rol    $0x6,%edi
     4f3:	31 fb                	xor    %edi,%ebx
     4f5:	89 df                	mov    %ebx,%edi
     4f7:	29 d8                	sub    %ebx,%eax
     4f9:	01 f3                	add    %esi,%ebx
     4fb:	c1 c7 08             	rol    $0x8,%edi
     4fe:	31 f8                	xor    %edi,%eax
     500:	89 c7                	mov    %eax,%edi
     502:	29 c6                	sub    %eax,%esi
     504:	01 d8                	add    %ebx,%eax
     506:	c1 c7 10             	rol    $0x10,%edi
     509:	31 fe                	xor    %edi,%esi
     50b:	89 f7                	mov    %esi,%edi
     50d:	29 f3                	sub    %esi,%ebx
     50f:	01 c6                	add    %eax,%esi
     511:	c1 cf 0d             	ror    $0xd,%edi
     514:	31 fb                	xor    %edi,%ebx
     516:	89 df                	mov    %ebx,%edi
     518:	29 d8                	sub    %ebx,%eax
     51a:	01 f3                	add    %esi,%ebx
     51c:	c1 c7 04             	rol    $0x4,%edi
     51f:	31 f8                	xor    %edi,%eax
	const unsigned char* key = tommy_cast(const unsigned char*,void_key);
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + init_val;

	while (key_len > 12) {
     521:	83 f9 0c             	cmp    $0xc,%ecx
     524:	77 aa                	ja     4d0 <tommy_hash_u32+0x20>

		key_len -= 12;
		key += 12;
	}

	switch (key_len) {
     526:	ff 24 8d 00 00 00 00 	jmp    *0x0(,%ecx,4)
	case 12 :
		c += tommy_le_uint32_read(key + 8);
		b += tommy_le_uint32_read(key + 4);
		a += tommy_le_uint32_read(key + 0);
		break;
	case 11 : c += ((tommy_uint32_t)key[10]) << 16;
     52d:	0f b6 4a 0a          	movzbl 0xa(%edx),%ecx
     531:	c1 e1 10             	shl    $0x10,%ecx
     534:	01 c8                	add    %ecx,%eax
	case 10 : c += ((tommy_uint32_t)key[9]) << 8;
     536:	0f b6 4a 09          	movzbl 0x9(%edx),%ecx
     53a:	c1 e1 08             	shl    $0x8,%ecx
     53d:	01 c8                	add    %ecx,%eax
	case 9 : c += key[8];
     53f:	0f b6 4a 08          	movzbl 0x8(%edx),%ecx
     543:	01 c8                	add    %ecx,%eax
	case 8 :
		b += tommy_le_uint32_read(key + 4);
     545:	03 5a 04             	add    0x4(%edx),%ebx
		a += tommy_le_uint32_read(key + 0);
     548:	03 32                	add    (%edx),%esi
	case 3 : a += ((tommy_uint32_t)key[2]) << 16;
	case 2 : a += ((tommy_uint32_t)key[1]) << 8;
	case 1 : a += key[0];
	}

	tommy_final(a,b,c);
     54a:	89 da                	mov    %ebx,%edx
     54c:	31 d8                	xor    %ebx,%eax
     54e:	c1 c2 0e             	rol    $0xe,%edx
     551:	29 d0                	sub    %edx,%eax
     553:	89 c2                	mov    %eax,%edx
     555:	31 c6                	xor    %eax,%esi
     557:	c1 c2 0b             	rol    $0xb,%edx
     55a:	29 d6                	sub    %edx,%esi
     55c:	89 f2                	mov    %esi,%edx
     55e:	31 f3                	xor    %esi,%ebx
     560:	c1 ca 07             	ror    $0x7,%edx
     563:	29 d3                	sub    %edx,%ebx
     565:	89 da                	mov    %ebx,%edx
     567:	31 d8                	xor    %ebx,%eax
     569:	c1 c2 10             	rol    $0x10,%edx
     56c:	29 d0                	sub    %edx,%eax
     56e:	89 c2                	mov    %eax,%edx
     570:	31 c6                	xor    %eax,%esi
     572:	c1 c2 04             	rol    $0x4,%edx
     575:	29 d6                	sub    %edx,%esi
     577:	31 f3                	xor    %esi,%ebx
     579:	c1 c6 0e             	rol    $0xe,%esi
     57c:	29 f3                	sub    %esi,%ebx
     57e:	31 d8                	xor    %ebx,%eax
     580:	c1 cb 08             	ror    $0x8,%ebx

	return c;
     583:	29 d8                	sub    %ebx,%eax
}
     585:	5b                   	pop    %ebx
     586:	5e                   	pop    %esi
     587:	5f                   	pop    %edi
     588:	c3                   	ret    
	case 9 : c += key[8];
	case 8 :
		b += tommy_le_uint32_read(key + 4);
		a += tommy_le_uint32_read(key + 0);
		break;
	case 7 : b += ((tommy_uint32_t)key[6]) << 16;
     589:	0f b6 4a 06          	movzbl 0x6(%edx),%ecx
     58d:	c1 e1 10             	shl    $0x10,%ecx
     590:	01 cb                	add    %ecx,%ebx
	case 6 : b += ((tommy_uint32_t)key[5]) << 8;
     592:	0f b6 4a 05          	movzbl 0x5(%edx),%ecx
     596:	c1 e1 08             	shl    $0x8,%ecx
     599:	01 cb                	add    %ecx,%ebx
	case 5 : b += key[4];
     59b:	0f b6 4a 04          	movzbl 0x4(%edx),%ecx
     59f:	01 cb                	add    %ecx,%ebx
	case 4 :
		a += tommy_le_uint32_read(key + 0);
     5a1:	03 32                	add    (%edx),%esi
		break;
     5a3:	eb a5                	jmp    54a <tommy_hash_u32+0x9a>
	case 3 : a += ((tommy_uint32_t)key[2]) << 16;
     5a5:	0f b6 4a 02          	movzbl 0x2(%edx),%ecx
     5a9:	c1 e1 10             	shl    $0x10,%ecx
     5ac:	01 ce                	add    %ecx,%esi
	case 2 : a += ((tommy_uint32_t)key[1]) << 8;
     5ae:	0f b6 4a 01          	movzbl 0x1(%edx),%ecx
     5b2:	c1 e1 08             	shl    $0x8,%ecx
     5b5:	01 ce                	add    %ecx,%esi
	case 1 : a += key[0];
     5b7:	0f b6 12             	movzbl (%edx),%edx
     5ba:	01 d6                	add    %edx,%esi
     5bc:	eb 8c                	jmp    54a <tommy_hash_u32+0x9a>

	switch (key_len) {
	case 0 :
		return c; /* used only when called with a zero length */
	case 12 :
		c += tommy_le_uint32_read(key + 8);
     5be:	03 42 08             	add    0x8(%edx),%eax
		b += tommy_le_uint32_read(key + 4);
     5c1:	03 5a 04             	add    0x4(%edx),%ebx
		a += tommy_le_uint32_read(key + 0);
     5c4:	03 32                	add    (%edx),%esi
		break;
     5c6:	eb 82                	jmp    54a <tommy_hash_u32+0x9a>
     5c8:	90                   	nop
     5c9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

000005d0 <tommy_hash_u64>:

	return c;
}

tommy_uint64_t tommy_hash_u64(tommy_uint64_t init_val, const void* void_key, tommy_size_t key_len)
{
     5d0:	57                   	push   %edi
     5d1:	56                   	push   %esi
     5d2:	53                   	push   %ebx
     5d3:	8b 74 24 1c          	mov    0x1c(%esp),%esi
	const unsigned char* key = tommy_cast(const unsigned char*,void_key);
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + (init_val & 0xffffffff);
	c += init_val >> 32;
     5d7:	8b 4c 24 14          	mov    0x14(%esp),%ecx

	return c;
}

tommy_uint64_t tommy_hash_u64(tommy_uint64_t init_val, const void* void_key, tommy_size_t key_len)
{
     5db:	8b 5c 24 18          	mov    0x18(%esp),%ebx
     5df:	8d be ef be ad de    	lea    -0x21524111(%esi),%edi
	const unsigned char* key = tommy_cast(const unsigned char*,void_key);
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + (init_val & 0xffffffff);
     5e5:	03 7c 24 10          	add    0x10(%esp),%edi
	c += init_val >> 32;
     5e9:	01 f9                	add    %edi,%ecx

	while (key_len > 12) {
     5eb:	83 fe 0c             	cmp    $0xc,%esi
     5ee:	89 f8                	mov    %edi,%eax
     5f0:	76 64                	jbe    656 <tommy_hash_u64+0x86>
     5f2:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
     5f8:	03 43 04             	add    0x4(%ebx),%eax
		c += tommy_le_uint32_read(key + 8);

		tommy_mix(a,b,c);

		key_len -= 12;
     5fb:	83 ee 0c             	sub    $0xc,%esi
		key += 12;
     5fe:	83 c3 0c             	add    $0xc,%ebx
	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + (init_val & 0xffffffff);
	c += init_val >> 32;

	while (key_len > 12) {
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
     601:	89 c2                	mov    %eax,%edx
		c += tommy_le_uint32_read(key + 8);
     603:	8b 43 fc             	mov    -0x4(%ebx),%eax
     606:	01 c8                	add    %ecx,%eax
     608:	29 c7                	sub    %eax,%edi

		tommy_mix(a,b,c);
     60a:	03 7b f4             	add    -0xc(%ebx),%edi
     60d:	89 c1                	mov    %eax,%ecx
     60f:	c1 c1 04             	rol    $0x4,%ecx
     612:	31 cf                	xor    %ecx,%edi
     614:	8d 0c 10             	lea    (%eax,%edx,1),%ecx
     617:	89 f8                	mov    %edi,%eax
     619:	29 fa                	sub    %edi,%edx
     61b:	c1 c0 06             	rol    $0x6,%eax
     61e:	01 cf                	add    %ecx,%edi
     620:	31 c2                	xor    %eax,%edx
     622:	89 c8                	mov    %ecx,%eax
     624:	89 d1                	mov    %edx,%ecx
     626:	29 d0                	sub    %edx,%eax
     628:	01 fa                	add    %edi,%edx
     62a:	c1 c1 08             	rol    $0x8,%ecx
     62d:	31 c8                	xor    %ecx,%eax
     62f:	89 c1                	mov    %eax,%ecx
     631:	29 c7                	sub    %eax,%edi
     633:	01 d0                	add    %edx,%eax
     635:	c1 c1 10             	rol    $0x10,%ecx
     638:	31 cf                	xor    %ecx,%edi
     63a:	89 f9                	mov    %edi,%ecx
     63c:	29 fa                	sub    %edi,%edx
     63e:	01 c7                	add    %eax,%edi
     640:	c1 c9 0d             	ror    $0xd,%ecx
     643:	31 ca                	xor    %ecx,%edx
     645:	89 d1                	mov    %edx,%ecx
     647:	29 d0                	sub    %edx,%eax
     649:	c1 c1 04             	rol    $0x4,%ecx
     64c:	31 c1                	xor    %eax,%ecx
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + (init_val & 0xffffffff);
	c += init_val >> 32;

	while (key_len > 12) {
     64e:	83 fe 0c             	cmp    $0xc,%esi
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
		c += tommy_le_uint32_read(key + 8);

		tommy_mix(a,b,c);
     651:	8d 04 3a             	lea    (%edx,%edi,1),%eax
	tommy_uint32_t a, b, c;

	a = b = c = 0xdeadbeef + ((tommy_uint32_t)key_len) + (init_val & 0xffffffff);
	c += init_val >> 32;

	while (key_len > 12) {
     654:	77 a2                	ja     5f8 <tommy_hash_u64+0x28>

		key_len -= 12;
		key += 12;
	}

	switch (key_len) {
     656:	ff 24 b5 34 00 00 00 	jmp    *0x34(,%esi,4)
	case 12 :
		c += tommy_le_uint32_read(key + 8);
		b += tommy_le_uint32_read(key + 4);
		a += tommy_le_uint32_read(key + 0);
		break;
	case 11 : c += ((tommy_uint32_t)key[10]) << 16;
     65d:	0f b6 53 0a          	movzbl 0xa(%ebx),%edx
     661:	c1 e2 10             	shl    $0x10,%edx
     664:	01 d1                	add    %edx,%ecx
	case 10 : c += ((tommy_uint32_t)key[9]) << 8;
     666:	0f b6 53 09          	movzbl 0x9(%ebx),%edx
     66a:	c1 e2 08             	shl    $0x8,%edx
     66d:	01 d1                	add    %edx,%ecx
	case 9 : c += key[8];
     66f:	0f b6 53 08          	movzbl 0x8(%ebx),%edx
     673:	01 d1                	add    %edx,%ecx
	case 8 :
		b += tommy_le_uint32_read(key + 4);
     675:	03 43 04             	add    0x4(%ebx),%eax
		a += tommy_le_uint32_read(key + 0);
     678:	03 3b                	add    (%ebx),%edi
	case 3 : a += ((tommy_uint32_t)key[2]) << 16;
	case 2 : a += ((tommy_uint32_t)key[1]) << 8;
	case 1 : a += key[0];
	}

	tommy_final(a,b,c);
     67a:	31 c1                	xor    %eax,%ecx
     67c:	89 c2                	mov    %eax,%edx
     67e:	c1 c2 0e             	rol    $0xe,%edx
     681:	89 cb                	mov    %ecx,%ebx
     683:	29 d3                	sub    %edx,%ebx
     685:	89 da                	mov    %ebx,%edx
     687:	31 df                	xor    %ebx,%edi
     689:	c1 c2 0b             	rol    $0xb,%edx
     68c:	29 d7                	sub    %edx,%edi
     68e:	31 f8                	xor    %edi,%eax
     690:	89 c2                	mov    %eax,%edx
     692:	89 f8                	mov    %edi,%eax
     694:	c1 c8 07             	ror    $0x7,%eax
     697:	29 c2                	sub    %eax,%edx
     699:	31 d3                	xor    %edx,%ebx
     69b:	89 d0                	mov    %edx,%eax
     69d:	c1 c0 10             	rol    $0x10,%eax
     6a0:	89 d9                	mov    %ebx,%ecx

	return c + ((tommy_uint64_t)b << 32);
     6a2:	31 db                	xor    %ebx,%ebx
	case 3 : a += ((tommy_uint32_t)key[2]) << 16;
	case 2 : a += ((tommy_uint32_t)key[1]) << 8;
	case 1 : a += key[0];
	}

	tommy_final(a,b,c);
     6a4:	29 c1                	sub    %eax,%ecx
     6a6:	89 c8                	mov    %ecx,%eax
     6a8:	31 cf                	xor    %ecx,%edi
     6aa:	c1 c0 04             	rol    $0x4,%eax
     6ad:	29 c7                	sub    %eax,%edi
     6af:	31 fa                	xor    %edi,%edx
     6b1:	c1 c7 0e             	rol    $0xe,%edi
     6b4:	89 d0                	mov    %edx,%eax
     6b6:	29 f8                	sub    %edi,%eax
     6b8:	89 c2                	mov    %eax,%edx
     6ba:	31 c1                	xor    %eax,%ecx
     6bc:	c1 ca 08             	ror    $0x8,%edx
     6bf:	29 d1                	sub    %edx,%ecx

	return c + ((tommy_uint64_t)b << 32);
     6c1:	89 c2                	mov    %eax,%edx
     6c3:	31 c0                	xor    %eax,%eax
     6c5:	01 c8                	add    %ecx,%eax
     6c7:	11 da                	adc    %ebx,%edx
}
     6c9:	5b                   	pop    %ebx
     6ca:	5e                   	pop    %esi
     6cb:	5f                   	pop    %edi
     6cc:	c3                   	ret    
	case 9 : c += key[8];
	case 8 :
		b += tommy_le_uint32_read(key + 4);
		a += tommy_le_uint32_read(key + 0);
		break;
	case 7 : b += ((tommy_uint32_t)key[6]) << 16;
     6cd:	0f b6 53 06          	movzbl 0x6(%ebx),%edx
     6d1:	c1 e2 10             	shl    $0x10,%edx
     6d4:	01 d0                	add    %edx,%eax
	case 6 : b += ((tommy_uint32_t)key[5]) << 8;
     6d6:	0f b6 53 05          	movzbl 0x5(%ebx),%edx
     6da:	c1 e2 08             	shl    $0x8,%edx
     6dd:	01 d0                	add    %edx,%eax
	case 5 : b += key[4];
     6df:	0f b6 53 04          	movzbl 0x4(%ebx),%edx
     6e3:	01 d0                	add    %edx,%eax
	case 4 :
		a += tommy_le_uint32_read(key + 0);
     6e5:	03 3b                	add    (%ebx),%edi
		break;
     6e7:	eb 91                	jmp    67a <tommy_hash_u64+0xaa>
	case 3 : a += ((tommy_uint32_t)key[2]) << 16;
     6e9:	0f b6 53 02          	movzbl 0x2(%ebx),%edx
     6ed:	c1 e2 10             	shl    $0x10,%edx
     6f0:	01 d7                	add    %edx,%edi
	case 2 : a += ((tommy_uint32_t)key[1]) << 8;
     6f2:	0f b6 53 01          	movzbl 0x1(%ebx),%edx
     6f6:	c1 e2 08             	shl    $0x8,%edx
     6f9:	01 d7                	add    %edx,%edi
	case 1 : a += key[0];
     6fb:	0f b6 13             	movzbl (%ebx),%edx
     6fe:	01 d7                	add    %edx,%edi
     700:	e9 75 ff ff ff       	jmp    67a <tommy_hash_u64+0xaa>
		key += 12;
	}

	switch (key_len) {
	case 0 :
		return c + ((tommy_uint64_t)b << 32); /* used only when called with a zero length */
     705:	89 c2                	mov    %eax,%edx
     707:	31 c0                	xor    %eax,%eax
     709:	31 ff                	xor    %edi,%edi
     70b:	01 c8                	add    %ecx,%eax
     70d:	11 fa                	adc    %edi,%edx
     70f:	eb b8                	jmp    6c9 <tommy_hash_u64+0xf9>
	case 12 :
		c += tommy_le_uint32_read(key + 8);
     711:	03 4b 08             	add    0x8(%ebx),%ecx
		b += tommy_le_uint32_read(key + 4);
     714:	03 43 04             	add    0x4(%ebx),%eax
		a += tommy_le_uint32_read(key + 0);
     717:	03 3b                	add    (%ebx),%edi
		break;
     719:	e9 5c ff ff ff       	jmp    67a <tommy_hash_u64+0xaa>
     71e:	66 90                	xchg   %ax,%ax

00000720 <tommy_allocator_init>:
 * The heap manager may put it in a single memory page.
 */
#define TOMMY_ALLOCATOR_BLOCK_SIZE (4096-64)

void tommy_allocator_init(tommy_allocator* alloc, unsigned block_size, unsigned align_size)
{
     720:	56                   	push   %esi
     721:	53                   	push   %ebx
	/* setup the minimal alignment */
	if (align_size < sizeof(void*))
		align_size = sizeof(void*);
     722:	b8 04 00 00 00       	mov    $0x4,%eax
 * The heap manager may put it in a single memory page.
 */
#define TOMMY_ALLOCATOR_BLOCK_SIZE (4096-64)

void tommy_allocator_init(tommy_allocator* alloc, unsigned block_size, unsigned align_size)
{
     727:	8b 4c 24 14          	mov    0x14(%esp),%ecx
     72b:	8b 5c 24 10          	mov    0x10(%esp),%ebx
     72f:	8b 74 24 0c          	mov    0xc(%esp),%esi
	/* setup the minimal alignment */
	if (align_size < sizeof(void*))
		align_size = sizeof(void*);
     733:	83 f9 03             	cmp    $0x3,%ecx
     736:	0f 46 c8             	cmovbe %eax,%ecx

	/* ensure that the block_size keeps the alignment */
	if (block_size % align_size != 0) {
     739:	31 d2                	xor    %edx,%edx
     73b:	89 d8                	mov    %ebx,%eax
     73d:	f7 f1                	div    %ecx
     73f:	85 d2                	test   %edx,%edx
     741:	74 04                	je     747 <tommy_allocator_init+0x27>
     743:	01 cb                	add    %ecx,%ebx
		block_size += align_size - block_size % align_size;
     745:	29 d3                	sub    %edx,%ebx
	}

	alloc->block_size = block_size;
     747:	89 5e 08             	mov    %ebx,0x8(%esi)
	alloc->align_size = align_size;
     74a:	89 4e 0c             	mov    %ecx,0xc(%esi)

	alloc->count = 0;
     74d:	c7 46 10 00 00 00 00 	movl   $0x0,0x10(%esi)
	alloc->free_block = 0;
     754:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
	alloc->used_segment = 0;
     75a:	c7 46 04 00 00 00 00 	movl   $0x0,0x4(%esi)
}
     761:	5b                   	pop    %ebx
     762:	5e                   	pop    %esi
     763:	c3                   	ret    
     764:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     76a:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

00000770 <tommy_allocator_done>:
	alloc->free_block = 0;
	alloc->used_segment = 0;
}

void tommy_allocator_done(tommy_allocator* alloc)
{
     770:	56                   	push   %esi
     771:	53                   	push   %ebx
     772:	83 ec 04             	sub    $0x4,%esp
     775:	8b 74 24 10          	mov    0x10(%esp),%esi
/**
 * Reset the allocator and free all.
 */ 
static void allocator_reset(tommy_allocator* alloc)
{
	tommy_allocator_entry* block = alloc->used_segment;
     779:	8b 46 04             	mov    0x4(%esi),%eax
	while (block) {
     77c:	85 c0                	test   %eax,%eax
     77e:	74 14                	je     794 <tommy_allocator_done+0x24>
		tommy_allocator_entry* block_next = block->next;
     780:	8b 18                	mov    (%eax),%ebx
		tommy_free(block);
     782:	83 ec 0c             	sub    $0xc,%esp
     785:	50                   	push   %eax
     786:	e8 fc ff ff ff       	call   787 <tommy_allocator_done+0x17>
 * Reset the allocator and free all.
 */ 
static void allocator_reset(tommy_allocator* alloc)
{
	tommy_allocator_entry* block = alloc->used_segment;
	while (block) {
     78b:	83 c4 10             	add    $0x10,%esp
     78e:	85 db                	test   %ebx,%ebx
     790:	89 d8                	mov    %ebx,%eax
     792:	75 ec                	jne    780 <tommy_allocator_done+0x10>
		tommy_allocator_entry* block_next = block->next;
		tommy_free(block);
		block = block_next;
	}

	alloc->count = 0;
     794:	c7 46 10 00 00 00 00 	movl   $0x0,0x10(%esi)
	alloc->free_block = 0;
     79b:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
	alloc->used_segment = 0;
     7a1:	c7 46 04 00 00 00 00 	movl   $0x0,0x4(%esi)
}

void tommy_allocator_done(tommy_allocator* alloc)
{
	allocator_reset(alloc);
}
     7a8:	83 c4 04             	add    $0x4,%esp
     7ab:	5b                   	pop    %ebx
     7ac:	5e                   	pop    %esi
     7ad:	c3                   	ret    
     7ae:	66 90                	xchg   %ax,%ax

000007b0 <tommy_allocator_alloc>:

void* tommy_allocator_alloc(tommy_allocator* alloc)
{
     7b0:	55                   	push   %ebp
     7b1:	57                   	push   %edi
     7b2:	56                   	push   %esi
     7b3:	53                   	push   %ebx
     7b4:	83 ec 0c             	sub    $0xc,%esp
     7b7:	8b 74 24 20          	mov    0x20(%esp),%esi
	void* ptr;

	/* if no free block available */
	if (!alloc->free_block) {
     7bb:	8b 1e                	mov    (%esi),%ebx
     7bd:	85 db                	test   %ebx,%ebx
     7bf:	74 17                	je     7d8 <tommy_allocator_alloc+0x28>
     7c1:	8b 03                	mov    (%ebx),%eax
		}
	}

	/* remove one from the free list */
	ptr = alloc->free_block;
	alloc->free_block = alloc->free_block->next;
     7c3:	89 06                	mov    %eax,(%esi)
	
	++alloc->count;
     7c5:	83 46 10 01          	addl   $0x1,0x10(%esi)

	return ptr;
}
     7c9:	83 c4 0c             	add    $0xc,%esp
     7cc:	89 d8                	mov    %ebx,%eax
     7ce:	5b                   	pop    %ebx
     7cf:	5e                   	pop    %esi
     7d0:	5f                   	pop    %edi
     7d1:	5d                   	pop    %ebp
     7d2:	c3                   	ret    
     7d3:	90                   	nop
     7d4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

	/* if no free block available */
	if (!alloc->free_block) {
		tommy_uintptr_t off, mis;
		tommy_size_t size = TOMMY_ALLOCATOR_BLOCK_SIZE;
		char* data = tommy_cast(char*, tommy_malloc(size));
     7d8:	83 ec 0c             	sub    $0xc,%esp
     7db:	68 c0 0f 00 00       	push   $0xfc0
     7e0:	e8 fc ff ff ff       	call   7e1 <tommy_allocator_alloc+0x31>
		tommy_allocator_entry* segment = (tommy_allocator_entry*)data;

		/* put in the segment list */
		segment->next = alloc->used_segment;
     7e5:	8b 56 04             	mov    0x4(%esi),%edx
		alloc->used_segment = segment;
		data += sizeof(tommy_allocator_entry);
     7e8:	8d 48 04             	lea    0x4(%eax),%ecx

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
     7eb:	8b 7e 0c             	mov    0xc(%esi),%edi
		char* data = tommy_cast(char*, tommy_malloc(size));
		tommy_allocator_entry* segment = (tommy_allocator_entry*)data;

		/* put in the segment list */
		segment->next = alloc->used_segment;
		alloc->used_segment = segment;
     7ee:	89 46 04             	mov    %eax,0x4(%esi)
		data += sizeof(tommy_allocator_entry);

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
		if (mis != 0) {
     7f1:	83 c4 10             	add    $0x10,%esp
		tommy_size_t size = TOMMY_ALLOCATOR_BLOCK_SIZE;
		char* data = tommy_cast(char*, tommy_malloc(size));
		tommy_allocator_entry* segment = (tommy_allocator_entry*)data;

		/* put in the segment list */
		segment->next = alloc->used_segment;
     7f4:	89 10                	mov    %edx,(%eax)
		alloc->used_segment = segment;
		data += sizeof(tommy_allocator_entry);

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
     7f6:	31 d2                	xor    %edx,%edx
     7f8:	89 c8                	mov    %ecx,%eax
     7fa:	f7 f7                	div    %edi
	void* ptr;

	/* if no free block available */
	if (!alloc->free_block) {
		tommy_uintptr_t off, mis;
		tommy_size_t size = TOMMY_ALLOCATOR_BLOCK_SIZE;
     7fc:	b8 c0 0f 00 00       	mov    $0xfc0,%eax
		data += sizeof(tommy_allocator_entry);

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
		if (mis != 0) {
     801:	85 d2                	test   %edx,%edx
     803:	74 0a                	je     80f <tommy_allocator_alloc+0x5f>
			data += alloc->align_size - mis;
     805:	89 fd                	mov    %edi,%ebp
     807:	29 f8                	sub    %edi,%eax
     809:	29 d5                	sub    %edx,%ebp
			size -= alloc->align_size - mis;
     80b:	01 d0                	add    %edx,%eax

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
		if (mis != 0) {
			data += alloc->align_size - mis;
     80d:	01 e9                	add    %ebp,%ecx
			size -= alloc->align_size - mis;
		}

		/* insert in free list */
		while (size >= alloc->block_size) {
     80f:	8b 56 08             	mov    0x8(%esi),%edx
     812:	39 d0                	cmp    %edx,%eax
     814:	73 0e                	jae    824 <tommy_allocator_alloc+0x74>
     816:	eb 1d                	jmp    835 <tommy_allocator_alloc+0x85>
     818:	90                   	nop
     819:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     820:	89 cb                	mov    %ecx,%ebx
			tommy_allocator_entry* free_block = (tommy_allocator_entry*)data;
			free_block->next = alloc->free_block;
			alloc->free_block = free_block;

			data += alloc->block_size;
     822:	89 f9                	mov    %edi,%ecx
			size -= alloc->block_size;
     824:	29 d0                	sub    %edx,%eax
		}

		/* insert in free list */
		while (size >= alloc->block_size) {
			tommy_allocator_entry* free_block = (tommy_allocator_entry*)data;
			free_block->next = alloc->free_block;
     826:	89 19                	mov    %ebx,(%ecx)
			alloc->free_block = free_block;

			data += alloc->block_size;
     828:	8d 3c 11             	lea    (%ecx,%edx,1),%edi
			data += alloc->align_size - mis;
			size -= alloc->align_size - mis;
		}

		/* insert in free list */
		while (size >= alloc->block_size) {
     82b:	39 d0                	cmp    %edx,%eax
     82d:	73 f1                	jae    820 <tommy_allocator_alloc+0x70>
     82f:	89 d8                	mov    %ebx,%eax
     831:	89 cb                	mov    %ecx,%ebx
     833:	eb 8e                	jmp    7c3 <tommy_allocator_alloc+0x13>
     835:	a1 00 00 00 00       	mov    0x0,%eax
     83a:	eb 87                	jmp    7c3 <tommy_allocator_alloc+0x13>
     83c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00000840 <tommy_allocator_free>:

	return ptr;
}

void tommy_allocator_free(tommy_allocator* alloc, void* ptr)
{
     840:	8b 44 24 04          	mov    0x4(%esp),%eax
     844:	8b 54 24 08          	mov    0x8(%esp),%edx
	tommy_allocator_entry* free_block = tommy_cast(tommy_allocator_entry*, ptr);
	
	/* put it in the free list */
	free_block->next = alloc->free_block;
     848:	8b 08                	mov    (%eax),%ecx
     84a:	89 0a                	mov    %ecx,(%edx)
	alloc->free_block = free_block;
     84c:	89 10                	mov    %edx,(%eax)

	--alloc->count;
     84e:	83 68 10 01          	subl   $0x1,0x10(%eax)
     852:	c3                   	ret    
     853:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     859:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000860 <tommy_allocator_memory_usage>:
}

tommy_size_t tommy_allocator_memory_usage(tommy_allocator* alloc)
{
     860:	8b 54 24 04          	mov    0x4(%esp),%edx
	return alloc->count * (tommy_size_t)alloc->block_size;
     864:	8b 42 08             	mov    0x8(%edx),%eax
     867:	0f af 42 10          	imul   0x10(%edx),%eax
}
     86b:	c3                   	ret    
     86c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00000870 <tommy_array_init>:

/******************************************************************************/
/* array */

void tommy_array_init(tommy_array* array)
{
     870:	57                   	push   %edi
     871:	53                   	push   %ebx
     872:	83 ec 10             	sub    $0x10,%esp
     875:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
	/* fixed initial size */
	array->bucket_bit = TOMMY_ARRAY_BIT;
     879:	c7 83 80 00 00 00 06 	movl   $0x6,0x80(%ebx)
     880:	00 00 00 
	array->bucket_max = 1 << array->bucket_bit;
     883:	c7 83 84 00 00 00 40 	movl   $0x40,0x84(%ebx)
     88a:	00 00 00 
	array->bucket[0] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     88d:	68 00 01 00 00       	push   $0x100
     892:	e8 fc ff ff ff       	call   893 <tommy_array_init+0x23>
     897:	89 c2                	mov    %eax,%edx
     899:	89 03                	mov    %eax,(%ebx)

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * sizeof(void*));
     89b:	b9 40 00 00 00       	mov    $0x40,%ecx
     8a0:	31 c0                	xor    %eax,%eax
     8a2:	89 d7                	mov    %edx,%edi

	array->bucket_mac = 1;
     8a4:	c7 83 88 00 00 00 01 	movl   $0x1,0x88(%ebx)
     8ab:	00 00 00 
	array->bucket_bit = TOMMY_ARRAY_BIT;
	array->bucket_max = 1 << array->bucket_bit;
	array->bucket[0] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * sizeof(void*));
     8ae:	f3 ab                	rep stos %eax,%es:(%edi)

	array->bucket_mac = 1;
	array->size = 0;
     8b0:	c7 83 8c 00 00 00 00 	movl   $0x0,0x8c(%ebx)
     8b7:	00 00 00 
}
     8ba:	83 c4 14             	add    $0x14,%esp
     8bd:	5b                   	pop    %ebx
     8be:	5f                   	pop    %edi
     8bf:	c3                   	ret    

000008c0 <tommy_array_done>:

void tommy_array_done(tommy_array* array)
{
     8c0:	56                   	push   %esi
     8c1:	53                   	push   %ebx
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     8c2:	31 db                	xor    %ebx,%ebx
	array->bucket_mac = 1;
	array->size = 0;
}

void tommy_array_done(tommy_array* array)
{
     8c4:	83 ec 04             	sub    $0x4,%esp
     8c7:	8b 74 24 10          	mov    0x10(%esp),%esi
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     8cb:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
     8d1:	85 c0                	test   %eax,%eax
     8d3:	74 1c                	je     8f1 <tommy_array_done+0x31>
     8d5:	8d 76 00             	lea    0x0(%esi),%esi
		tommy_free(array->bucket[i]);
     8d8:	83 ec 0c             	sub    $0xc,%esp
     8db:	ff 34 9e             	pushl  (%esi,%ebx,4)
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     8de:	83 c3 01             	add    $0x1,%ebx
		tommy_free(array->bucket[i]);
     8e1:	e8 fc ff ff ff       	call   8e2 <tommy_array_done+0x22>
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     8e6:	83 c4 10             	add    $0x10,%esp
     8e9:	39 9e 88 00 00 00    	cmp    %ebx,0x88(%esi)
     8ef:	77 e7                	ja     8d8 <tommy_array_done+0x18>
		tommy_free(array->bucket[i]);
}
     8f1:	83 c4 04             	add    $0x4,%esp
     8f4:	5b                   	pop    %ebx
     8f5:	5e                   	pop    %esi
     8f6:	c3                   	ret    
     8f7:	89 f6                	mov    %esi,%esi
     8f9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000900 <tommy_array_grow>:

void tommy_array_grow(tommy_array* array, unsigned size)
{
     900:	55                   	push   %ebp
     901:	57                   	push   %edi
     902:	56                   	push   %esi
     903:	53                   	push   %ebx
     904:	83 ec 1c             	sub    $0x1c,%esp
     907:	8b 74 24 30          	mov    0x30(%esp),%esi
	while (size > array->bucket_max) {
     90b:	8b 9e 84 00 00 00    	mov    0x84(%esi),%ebx
     911:	39 5c 24 34          	cmp    %ebx,0x34(%esp)
     915:	76 74                	jbe    98b <tommy_array_grow+0x8b>
     917:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
     91d:	8b 8e 80 00 00 00    	mov    0x80(%esi),%ecx
     923:	89 c7                	mov    %eax,%edi
     925:	89 cd                	mov    %ecx,%ebp
     927:	29 cf                	sub    %ecx,%edi
     929:	8d 14 be             	lea    (%esi,%edi,4),%edx
     92c:	8d 78 01             	lea    0x1(%eax),%edi
     92f:	29 cf                	sub    %ecx,%edi
     931:	89 54 24 0c          	mov    %edx,0xc(%esp)
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     935:	89 f8                	mov    %edi,%eax
     937:	89 f7                	mov    %esi,%edi
     939:	89 c6                	mov    %eax,%esi
     93b:	90                   	nop
     93c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     940:	83 ec 0c             	sub    $0xc,%esp
     943:	c1 e3 02             	shl    $0x2,%ebx
     946:	53                   	push   %ebx
     947:	e8 fc ff ff ff       	call   948 <tommy_array_grow+0x48>
     94c:	8b 54 24 1c          	mov    0x1c(%esp),%edx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     950:	83 c4 0c             	add    $0xc,%esp

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     953:	89 04 aa             	mov    %eax,(%edx,%ebp,4)

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     956:	53                   	push   %ebx

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     957:	bb 01 00 00 00       	mov    $0x1,%ebx
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     95c:	6a 00                	push   $0x0
     95e:	50                   	push   %eax
     95f:	e8 fc ff ff ff       	call   960 <tommy_array_grow+0x60>
     964:	8d 04 2e             	lea    (%esi,%ebp,1),%eax

		++array->bucket_mac;
		++array->bucket_bit;
     967:	83 c5 01             	add    $0x1,%ebp
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     96a:	83 c4 10             	add    $0x10,%esp
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     96d:	89 e9                	mov    %ebp,%ecx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
     96f:	89 af 80 00 00 00    	mov    %ebp,0x80(%edi)
		array->bucket_max = 1 << array->bucket_bit;
     975:	d3 e3                	shl    %cl,%ebx
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     977:	39 5c 24 34          	cmp    %ebx,0x34(%esp)
     97b:	89 87 88 00 00 00    	mov    %eax,0x88(%edi)
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     981:	89 9f 84 00 00 00    	mov    %ebx,0x84(%edi)
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     987:	77 b7                	ja     940 <tommy_array_grow+0x40>
     989:	89 fe                	mov    %edi,%esi
		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
	}

	if (array->size < size)
     98b:	8b 86 8c 00 00 00    	mov    0x8c(%esi),%eax
     991:	39 44 24 34          	cmp    %eax,0x34(%esp)
     995:	76 0a                	jbe    9a1 <tommy_array_grow+0xa1>
		array->size = size;
     997:	8b 44 24 34          	mov    0x34(%esp),%eax
     99b:	89 86 8c 00 00 00    	mov    %eax,0x8c(%esi)
}
     9a1:	83 c4 1c             	add    $0x1c,%esp
     9a4:	5b                   	pop    %ebx
     9a5:	5e                   	pop    %esi
     9a6:	5f                   	pop    %edi
     9a7:	5d                   	pop    %ebp
     9a8:	c3                   	ret    
     9a9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

000009b0 <tommy_array_memory_usage>:

tommy_size_t tommy_array_memory_usage(tommy_array* array)
{
	return array->bucket_max * (tommy_size_t)sizeof(void*);
     9b0:	8b 44 24 04          	mov    0x4(%esp),%eax
     9b4:	8b 80 84 00 00 00    	mov    0x84(%eax),%eax
     9ba:	c1 e0 02             	shl    $0x2,%eax
}
     9bd:	c3                   	ret    
     9be:	66 90                	xchg   %ax,%ax

000009c0 <tommy_arrayof_init>:

/******************************************************************************/
/* array */

void tommy_arrayof_init(tommy_arrayof* array, unsigned element_size)
{
     9c0:	56                   	push   %esi
     9c1:	53                   	push   %ebx
     9c2:	83 ec 10             	sub    $0x10,%esp
     9c5:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
     9c9:	8b 74 24 20          	mov    0x20(%esp),%esi
	/* fixed initial size */
	array->element_size = element_size;
     9cd:	89 b3 80 00 00 00    	mov    %esi,0x80(%ebx)
	array->bucket_bit = TOMMY_ARRAYOF_BIT;
	array->bucket_max = 1 << array->bucket_bit;
	array->bucket[0] = tommy_malloc(array->bucket_max * array->element_size);
     9d3:	c1 e6 06             	shl    $0x6,%esi

void tommy_arrayof_init(tommy_arrayof* array, unsigned element_size)
{
	/* fixed initial size */
	array->element_size = element_size;
	array->bucket_bit = TOMMY_ARRAYOF_BIT;
     9d6:	c7 83 84 00 00 00 06 	movl   $0x6,0x84(%ebx)
     9dd:	00 00 00 
	array->bucket_max = 1 << array->bucket_bit;
     9e0:	c7 83 88 00 00 00 40 	movl   $0x40,0x88(%ebx)
     9e7:	00 00 00 
	array->bucket[0] = tommy_malloc(array->bucket_max * array->element_size);
     9ea:	56                   	push   %esi
     9eb:	e8 fc ff ff ff       	call   9ec <tommy_arrayof_init+0x2c>

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * array->element_size);
     9f0:	83 c4 0c             	add    $0xc,%esp
{
	/* fixed initial size */
	array->element_size = element_size;
	array->bucket_bit = TOMMY_ARRAYOF_BIT;
	array->bucket_max = 1 << array->bucket_bit;
	array->bucket[0] = tommy_malloc(array->bucket_max * array->element_size);
     9f3:	89 03                	mov    %eax,(%ebx)

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * array->element_size);
     9f5:	56                   	push   %esi
     9f6:	6a 00                	push   $0x0
     9f8:	50                   	push   %eax
     9f9:	e8 fc ff ff ff       	call   9fa <tommy_arrayof_init+0x3a>

	array->bucket_mac = 1;
     9fe:	c7 83 8c 00 00 00 01 	movl   $0x1,0x8c(%ebx)
     a05:	00 00 00 
	array->size = 0;
     a08:	c7 83 90 00 00 00 00 	movl   $0x0,0x90(%ebx)
     a0f:	00 00 00 
}
     a12:	83 c4 14             	add    $0x14,%esp
     a15:	5b                   	pop    %ebx
     a16:	5e                   	pop    %esi
     a17:	c3                   	ret    
     a18:	90                   	nop
     a19:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00000a20 <tommy_arrayof_done>:

void tommy_arrayof_done(tommy_arrayof* array)
{
     a20:	56                   	push   %esi
     a21:	53                   	push   %ebx
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     a22:	31 db                	xor    %ebx,%ebx
	array->bucket_mac = 1;
	array->size = 0;
}

void tommy_arrayof_done(tommy_arrayof* array)
{
     a24:	83 ec 04             	sub    $0x4,%esp
     a27:	8b 74 24 10          	mov    0x10(%esp),%esi
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     a2b:	8b 86 8c 00 00 00    	mov    0x8c(%esi),%eax
     a31:	85 c0                	test   %eax,%eax
     a33:	74 1c                	je     a51 <tommy_arrayof_done+0x31>
     a35:	8d 76 00             	lea    0x0(%esi),%esi
		tommy_free(array->bucket[i]);
     a38:	83 ec 0c             	sub    $0xc,%esp
     a3b:	ff 34 9e             	pushl  (%esi,%ebx,4)
}

void tommy_arrayof_done(tommy_arrayof* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     a3e:	83 c3 01             	add    $0x1,%ebx
		tommy_free(array->bucket[i]);
     a41:	e8 fc ff ff ff       	call   a42 <tommy_arrayof_done+0x22>
}

void tommy_arrayof_done(tommy_arrayof* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     a46:	83 c4 10             	add    $0x10,%esp
     a49:	39 9e 8c 00 00 00    	cmp    %ebx,0x8c(%esi)
     a4f:	77 e7                	ja     a38 <tommy_arrayof_done+0x18>
		tommy_free(array->bucket[i]);
}
     a51:	83 c4 04             	add    $0x4,%esp
     a54:	5b                   	pop    %ebx
     a55:	5e                   	pop    %esi
     a56:	c3                   	ret    
     a57:	89 f6                	mov    %esi,%esi
     a59:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000a60 <tommy_arrayof_grow>:

void tommy_arrayof_grow(tommy_arrayof* array, unsigned size)
{
     a60:	55                   	push   %ebp
     a61:	57                   	push   %edi
     a62:	56                   	push   %esi
     a63:	53                   	push   %ebx
     a64:	83 ec 1c             	sub    $0x1c,%esp
     a67:	8b 74 24 30          	mov    0x30(%esp),%esi
	while (size > array->bucket_max) {
     a6b:	8b 9e 88 00 00 00    	mov    0x88(%esi),%ebx
     a71:	39 5c 24 34          	cmp    %ebx,0x34(%esp)
     a75:	76 7e                	jbe    af5 <tommy_arrayof_grow+0x95>
     a77:	8b 86 8c 00 00 00    	mov    0x8c(%esi),%eax
     a7d:	8b 8e 84 00 00 00    	mov    0x84(%esi),%ecx
     a83:	8b 96 80 00 00 00    	mov    0x80(%esi),%edx
     a89:	89 c7                	mov    %eax,%edi
     a8b:	89 cd                	mov    %ecx,%ebp
     a8d:	29 cf                	sub    %ecx,%edi
     a8f:	89 54 24 0c          	mov    %edx,0xc(%esp)
     a93:	8d 14 be             	lea    (%esi,%edi,4),%edx
     a96:	8d 78 01             	lea    0x1(%eax),%edi
     a99:	29 cf                	sub    %ecx,%edi
     a9b:	89 54 24 08          	mov    %edx,0x8(%esp)
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     a9f:	89 f8                	mov    %edi,%eax
     aa1:	89 f7                	mov    %esi,%edi
     aa3:	89 c6                	mov    %eax,%esi
     aa5:	8d 76 00             	lea    0x0(%esi),%esi

void tommy_arrayof_grow(tommy_arrayof* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_malloc(array->bucket_max * array->element_size);
     aa8:	0f af 5c 24 0c       	imul   0xc(%esp),%ebx
     aad:	83 ec 0c             	sub    $0xc,%esp
     ab0:	53                   	push   %ebx
     ab1:	e8 fc ff ff ff       	call   ab2 <tommy_arrayof_grow+0x52>
     ab6:	8b 54 24 18          	mov    0x18(%esp),%edx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);
     aba:	83 c4 0c             	add    $0xc,%esp

void tommy_arrayof_grow(tommy_arrayof* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_malloc(array->bucket_max * array->element_size);
     abd:	89 04 aa             	mov    %eax,(%edx,%ebp,4)

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);
     ac0:	53                   	push   %ebx

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     ac1:	bb 01 00 00 00       	mov    $0x1,%ebx
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_malloc(array->bucket_max * array->element_size);

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);
     ac6:	6a 00                	push   $0x0
     ac8:	50                   	push   %eax
     ac9:	e8 fc ff ff ff       	call   aca <tommy_arrayof_grow+0x6a>
     ace:	8d 04 2e             	lea    (%esi,%ebp,1),%eax

		++array->bucket_mac;
		++array->bucket_bit;
     ad1:	83 c5 01             	add    $0x1,%ebp
		tommy_free(array->bucket[i]);
}

void tommy_arrayof_grow(tommy_arrayof* array, unsigned size)
{
	while (size > array->bucket_max) {
     ad4:	83 c4 10             	add    $0x10,%esp
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     ad7:	89 e9                	mov    %ebp,%ecx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);

		++array->bucket_mac;
		++array->bucket_bit;
     ad9:	89 af 84 00 00 00    	mov    %ebp,0x84(%edi)
		array->bucket_max = 1 << array->bucket_bit;
     adf:	d3 e3                	shl    %cl,%ebx
		tommy_free(array->bucket[i]);
}

void tommy_arrayof_grow(tommy_arrayof* array, unsigned size)
{
	while (size > array->bucket_max) {
     ae1:	39 5c 24 34          	cmp    %ebx,0x34(%esp)
     ae5:	89 87 8c 00 00 00    	mov    %eax,0x8c(%edi)
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * array->element_size);

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     aeb:	89 9f 88 00 00 00    	mov    %ebx,0x88(%edi)
		tommy_free(array->bucket[i]);
}

void tommy_arrayof_grow(tommy_arrayof* array, unsigned size)
{
	while (size > array->bucket_max) {
     af1:	77 b5                	ja     aa8 <tommy_arrayof_grow+0x48>
     af3:	89 fe                	mov    %edi,%esi
		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
	}

	if (array->size < size)
     af5:	8b 86 90 00 00 00    	mov    0x90(%esi),%eax
     afb:	39 44 24 34          	cmp    %eax,0x34(%esp)
     aff:	76 0a                	jbe    b0b <tommy_arrayof_grow+0xab>
		array->size = size;
     b01:	8b 44 24 34          	mov    0x34(%esp),%eax
     b05:	89 86 90 00 00 00    	mov    %eax,0x90(%esi)
}
     b0b:	83 c4 1c             	add    $0x1c,%esp
     b0e:	5b                   	pop    %ebx
     b0f:	5e                   	pop    %esi
     b10:	5f                   	pop    %edi
     b11:	5d                   	pop    %ebp
     b12:	c3                   	ret    
     b13:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     b19:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000b20 <tommy_arrayof_memory_usage>:

tommy_size_t tommy_arrayof_memory_usage(tommy_arrayof* array)
{
     b20:	8b 54 24 04          	mov    0x4(%esp),%edx
	return array->bucket_max * (tommy_size_t)array->element_size;
     b24:	8b 82 80 00 00 00    	mov    0x80(%edx),%eax
     b2a:	0f af 82 88 00 00 00 	imul   0x88(%edx),%eax
}
     b31:	c3                   	ret    
     b32:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     b39:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000b40 <tommy_arrayblk_init>:

/******************************************************************************/
/* array */

void tommy_arrayblk_init(tommy_arrayblk* array)
{
     b40:	57                   	push   %edi
     b41:	53                   	push   %ebx
     b42:	83 ec 10             	sub    $0x10,%esp
     b45:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
/* array */

void tommy_array_init(tommy_array* array)
{
	/* fixed initial size */
	array->bucket_bit = TOMMY_ARRAY_BIT;
     b49:	c7 83 80 00 00 00 06 	movl   $0x6,0x80(%ebx)
     b50:	00 00 00 
	array->bucket_max = 1 << array->bucket_bit;
     b53:	c7 83 84 00 00 00 40 	movl   $0x40,0x84(%ebx)
     b5a:	00 00 00 
	array->bucket[0] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     b5d:	68 00 01 00 00       	push   $0x100
     b62:	e8 fc ff ff ff       	call   b63 <tommy_arrayblk_init+0x23>
     b67:	89 c2                	mov    %eax,%edx
     b69:	89 03                	mov    %eax,(%ebx)

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * sizeof(void*));
     b6b:	b9 40 00 00 00       	mov    $0x40,%ecx
     b70:	31 c0                	xor    %eax,%eax
     b72:	89 d7                	mov    %edx,%edi

	array->bucket_mac = 1;
     b74:	c7 83 88 00 00 00 01 	movl   $0x1,0x88(%ebx)
     b7b:	00 00 00 
	array->bucket_bit = TOMMY_ARRAY_BIT;
	array->bucket_max = 1 << array->bucket_bit;
	array->bucket[0] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * sizeof(void*));
     b7e:	f3 ab                	rep stos %eax,%es:(%edi)

	array->bucket_mac = 1;
	array->size = 0;
     b80:	c7 83 8c 00 00 00 00 	movl   $0x0,0x8c(%ebx)
     b87:	00 00 00 
	tommy_array_init(&array->block);

	array->size = 0;
     b8a:	c7 83 90 00 00 00 00 	movl   $0x0,0x90(%ebx)
     b91:	00 00 00 
}
     b94:	83 c4 14             	add    $0x14,%esp
     b97:	5b                   	pop    %ebx
     b98:	5f                   	pop    %edi
     b99:	c3                   	ret    
     b9a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

00000ba0 <tommy_arrayblk_done>:

void tommy_arrayblk_done(tommy_arrayblk* array)
{
     ba0:	57                   	push   %edi
     ba1:	56                   	push   %esi

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
     ba2:	bf 01 00 00 00       	mov    $0x1,%edi
     ba7:	53                   	push   %ebx
     ba8:	8b 74 24 10          	mov    0x10(%esp),%esi
	unsigned i;

	for(i=0;i<tommy_array_size(&array->block);++i)
     bac:	31 db                	xor    %ebx,%ebx
     bae:	8b 96 8c 00 00 00    	mov    0x8c(%esi),%edx
     bb4:	85 d2                	test   %edx,%edx
     bb6:	75 32                	jne    bea <tommy_arrayblk_done+0x4a>
     bb8:	eb 46                	jmp    c00 <tommy_arrayblk_done+0x60>
     bba:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
	 * Where "x ^ 31 = 31 - x", but gcc does not optimize "31 - __builtin_clz(x)" to bsr(x),
	 * but generates 31 - (bsr(x) xor 31).
	 *
	 * So we write "__builtin_clz(x) ^ 31" instead of "31 - __builtin_clz(x)".
	 */
	return __builtin_clz(value) ^ 31;
     bc0:	0f bd cb             	bsr    %ebx,%ecx
     bc3:	89 f8                	mov    %edi,%eax
     bc5:	89 da                	mov    %ebx,%edx
     bc7:	d3 e0                	shl    %cl,%eax
     bc9:	29 c2                	sub    %eax,%edx

	return &array->bucket[bsr - TOMMY_ARRAY_BIT + 1][pos];
     bcb:	8b 44 8e ec          	mov    -0x14(%esi,%ecx,4),%eax
     bcf:	8d 04 90             	lea    (%eax,%edx,4),%eax
		tommy_free(tommy_array_get(&array->block, i));
     bd2:	83 ec 0c             	sub    $0xc,%esp
     bd5:	ff 30                	pushl  (%eax)

void tommy_arrayblk_done(tommy_arrayblk* array)
{
	unsigned i;

	for(i=0;i<tommy_array_size(&array->block);++i)
     bd7:	83 c3 01             	add    $0x1,%ebx
		tommy_free(tommy_array_get(&array->block, i));
     bda:	e8 fc ff ff ff       	call   bdb <tommy_arrayblk_done+0x3b>

void tommy_arrayblk_done(tommy_arrayblk* array)
{
	unsigned i;

	for(i=0;i<tommy_array_size(&array->block);++i)
     bdf:	83 c4 10             	add    $0x10,%esp
     be2:	39 9e 8c 00 00 00    	cmp    %ebx,0x8c(%esi)
     be8:	76 16                	jbe    c00 <tommy_arrayblk_done+0x60>
	unsigned bsr;  

	assert(pos < array->size);

	/* special case for the first bucket */
	if (pos < (1 << TOMMY_ARRAY_BIT)) {
     bea:	83 fb 3f             	cmp    $0x3f,%ebx
     bed:	77 d1                	ja     bc0 <tommy_arrayblk_done+0x20>
		return &array->bucket[0][pos];
     bef:	8b 06                	mov    (%esi),%eax
     bf1:	8d 04 98             	lea    (%eax,%ebx,4),%eax
     bf4:	eb dc                	jmp    bd2 <tommy_arrayblk_done+0x32>
     bf6:	8d 76 00             	lea    0x0(%esi),%esi
     bf9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     c00:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
     c06:	31 db                	xor    %ebx,%ebx
     c08:	85 c0                	test   %eax,%eax
     c0a:	74 1d                	je     c29 <tommy_arrayblk_done+0x89>
     c0c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
		tommy_free(array->bucket[i]);
     c10:	83 ec 0c             	sub    $0xc,%esp
     c13:	ff 34 9e             	pushl  (%esi,%ebx,4)
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     c16:	83 c3 01             	add    $0x1,%ebx
		tommy_free(array->bucket[i]);
     c19:	e8 fc ff ff ff       	call   c1a <tommy_arrayblk_done+0x7a>
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     c1e:	83 c4 10             	add    $0x10,%esp
     c21:	3b 9e 88 00 00 00    	cmp    0x88(%esi),%ebx
     c27:	72 e7                	jb     c10 <tommy_arrayblk_done+0x70>
		tommy_free(tommy_array_get(&array->block, i));

	tommy_array_done(&array->block);
}
     c29:	5b                   	pop    %ebx
     c2a:	5e                   	pop    %esi
     c2b:	5f                   	pop    %edi
     c2c:	c3                   	ret    
     c2d:	8d 76 00             	lea    0x0(%esi),%esi

00000c30 <tommy_arrayblk_grow>:

void tommy_arrayblk_grow(tommy_arrayblk* array, unsigned size)
{
     c30:	55                   	push   %ebp
     c31:	57                   	push   %edi
     c32:	56                   	push   %esi
     c33:	53                   	push   %ebx
     c34:	83 ec 1c             	sub    $0x1c,%esp
     c37:	8b 74 24 30          	mov    0x30(%esp),%esi
	unsigned block_max = (size + TOMMY_ARRAYBLK_SIZE - 1) / TOMMY_ARRAYBLK_SIZE;
     c3b:	8b 44 24 34          	mov    0x34(%esp),%eax
     c3f:	8b 9e 8c 00 00 00    	mov    0x8c(%esi),%ebx
     c45:	8d b8 ff ff 00 00    	lea    0xffff(%eax),%edi
     c4b:	c1 ef 10             	shr    $0x10,%edi
	unsigned block_mac = tommy_array_size(&array->block);

	if (block_mac < block_max) {
     c4e:	39 df                	cmp    %ebx,%edi
     c50:	77 1e                	ja     c70 <tommy_arrayblk_grow+0x40>

			++block_mac;
		}
	}

	if (array->size < size)
     c52:	8b 86 90 00 00 00    	mov    0x90(%esi),%eax
     c58:	39 44 24 34          	cmp    %eax,0x34(%esp)
     c5c:	76 0a                	jbe    c68 <tommy_arrayblk_grow+0x38>
		array->size = size;
     c5e:	8b 44 24 34          	mov    0x34(%esp),%eax
     c62:	89 86 90 00 00 00    	mov    %eax,0x90(%esi)
}
     c68:	83 c4 1c             	add    $0x1c,%esp
     c6b:	5b                   	pop    %ebx
     c6c:	5e                   	pop    %esi
     c6d:	5f                   	pop    %edi
     c6e:	5d                   	pop    %ebp
     c6f:	c3                   	ret    
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     c70:	8b 96 84 00 00 00    	mov    0x84(%esi),%edx
     c76:	39 d7                	cmp    %edx,%edi
     c78:	0f 86 82 00 00 00    	jbe    d00 <tommy_arrayblk_grow+0xd0>
     c7e:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
     c84:	8b 8e 80 00 00 00    	mov    0x80(%esi),%ecx
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     c8a:	89 74 24 30          	mov    %esi,0x30(%esp)
     c8e:	89 5c 24 0c          	mov    %ebx,0xc(%esp)
     c92:	89 d3                	mov    %edx,%ebx
     c94:	89 c5                	mov    %eax,%ebp
     c96:	83 c0 01             	add    $0x1,%eax
     c99:	29 cd                	sub    %ecx,%ebp
     c9b:	29 c8                	sub    %ecx,%eax
     c9d:	8d 2c ae             	lea    (%esi,%ebp,4),%ebp
     ca0:	89 44 24 08          	mov    %eax,0x8(%esp)
     ca4:	89 ce                	mov    %ecx,%esi
     ca6:	89 6c 24 04          	mov    %ebp,0x4(%esp)
     caa:	8b 6c 24 30          	mov    0x30(%esp),%ebp
     cae:	66 90                	xchg   %ax,%ax

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     cb0:	83 ec 0c             	sub    $0xc,%esp
     cb3:	c1 e3 02             	shl    $0x2,%ebx
     cb6:	53                   	push   %ebx
     cb7:	e8 fc ff ff ff       	call   cb8 <tommy_arrayblk_grow+0x88>
     cbc:	8b 54 24 14          	mov    0x14(%esp),%edx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     cc0:	83 c4 0c             	add    $0xc,%esp

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     cc3:	89 04 b2             	mov    %eax,(%edx,%esi,4)

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     cc6:	53                   	push   %ebx

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     cc7:	bb 01 00 00 00       	mov    $0x1,%ebx
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     ccc:	6a 00                	push   $0x0
     cce:	50                   	push   %eax
     ccf:	e8 fc ff ff ff       	call   cd0 <tommy_arrayblk_grow+0xa0>
     cd4:	8b 44 24 18          	mov    0x18(%esp),%eax
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     cd8:	83 c4 10             	add    $0x10,%esp
     cdb:	01 f0                	add    %esi,%eax

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
     cdd:	83 c6 01             	add    $0x1,%esi
		array->bucket_max = 1 << array->bucket_bit;
     ce0:	89 f1                	mov    %esi,%ecx
     ce2:	89 85 88 00 00 00    	mov    %eax,0x88(%ebp)

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
     ce8:	89 b5 80 00 00 00    	mov    %esi,0x80(%ebp)
		array->bucket_max = 1 << array->bucket_bit;
     cee:	d3 e3                	shl    %cl,%ebx
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     cf0:	39 df                	cmp    %ebx,%edi
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     cf2:	89 9d 84 00 00 00    	mov    %ebx,0x84(%ebp)
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     cf8:	77 b6                	ja     cb0 <tommy_arrayblk_grow+0x80>
     cfa:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
     cfe:	89 ee                	mov    %ebp,%esi
     d00:	8d 04 9d 00 00 00 00 	lea    0x0(,%ebx,4),%eax
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
	}

	if (array->size < size)
		array->size = size;
     d07:	89 be 8c 00 00 00    	mov    %edi,0x8c(%esi)
     d0d:	89 74 24 30          	mov    %esi,0x30(%esp)
     d11:	89 c5                	mov    %eax,%ebp
     d13:	eb 2a                	jmp    d3f <tommy_arrayblk_grow+0x10f>
     d15:	8d 76 00             	lea    0x0(%esi),%esi
     d18:	0f bd cb             	bsr    %ebx,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
     d1b:	b8 01 00 00 00       	mov    $0x1,%eax
     d20:	89 da                	mov    %ebx,%edx
     d22:	d3 e0                	shl    %cl,%eax
			memset(ptr, 0, TOMMY_ARRAYBLK_SIZE * sizeof(void*));

			/* set the new block */
			tommy_array_set(&array->block, block_mac, ptr);

			++block_mac;
     d24:	83 c3 01             	add    $0x1,%ebx
     d27:	83 c5 04             	add    $0x4,%ebp
     d2a:	29 c2                	sub    %eax,%edx
	if (block_mac < block_max) {
		/* grow the block array */
		tommy_array_grow(&array->block, block_max);

		/* allocate new blocks */
		while (block_mac < block_max) {
     d2c:	39 fb                	cmp    %edi,%ebx
     d2e:	89 d0                	mov    %edx,%eax

	return &array->bucket[bsr - TOMMY_ARRAY_BIT + 1][pos];
     d30:	8b 54 24 30          	mov    0x30(%esp),%edx
     d34:	8b 4c 8a ec          	mov    -0x14(%edx,%ecx,4),%ecx
     d38:	8d 04 81             	lea    (%ecx,%eax,4),%eax
 * You must be sure that space for this position is already
 * allocated calling tommy_array_grow().
 */
tommy_inline void tommy_array_set(tommy_array* array, unsigned pos, void* element)
{
	*tommy_array_ref(array, pos) = element;
     d3b:	89 30                	mov    %esi,(%eax)
     d3d:	74 3f                	je     d7e <tommy_arrayblk_grow+0x14e>
			void* ptr = tommy_malloc(TOMMY_ARRAYBLK_SIZE * sizeof(void*));
     d3f:	83 ec 0c             	sub    $0xc,%esp
     d42:	68 00 00 04 00       	push   $0x40000
     d47:	e8 fc ff ff ff       	call   d48 <tommy_arrayblk_grow+0x118>

			/* initializes it with zeros */
			memset(ptr, 0, TOMMY_ARRAYBLK_SIZE * sizeof(void*));
     d4c:	83 c4 0c             	add    $0xc,%esp
		/* grow the block array */
		tommy_array_grow(&array->block, block_max);

		/* allocate new blocks */
		while (block_mac < block_max) {
			void* ptr = tommy_malloc(TOMMY_ARRAYBLK_SIZE * sizeof(void*));
     d4f:	89 c6                	mov    %eax,%esi

			/* initializes it with zeros */
			memset(ptr, 0, TOMMY_ARRAYBLK_SIZE * sizeof(void*));
     d51:	68 00 00 04 00       	push   $0x40000
     d56:	6a 00                	push   $0x0
     d58:	50                   	push   %eax
     d59:	e8 fc ff ff ff       	call   d5a <tommy_arrayblk_grow+0x12a>
 */
tommy_inline void** tommy_array_ref(tommy_array* array, unsigned pos)
{
	unsigned bsr;  

	assert(pos < array->size);
     d5e:	83 c4 10             	add    $0x10,%esp
     d61:	39 fb                	cmp    %edi,%ebx
     d63:	74 22                	je     d87 <tommy_arrayblk_grow+0x157>

	/* special case for the first bucket */
	if (pos < (1 << TOMMY_ARRAY_BIT)) {
     d65:	83 fb 3f             	cmp    $0x3f,%ebx
     d68:	77 ae                	ja     d18 <tommy_arrayblk_grow+0xe8>
		return &array->bucket[0][pos];
     d6a:	8b 4c 24 30          	mov    0x30(%esp),%ecx

			/* set the new block */
			tommy_array_set(&array->block, block_mac, ptr);

			++block_mac;
     d6e:	83 c3 01             	add    $0x1,%ebx
     d71:	8b 01                	mov    (%ecx),%eax
     d73:	01 e8                	add    %ebp,%eax
     d75:	83 c5 04             	add    $0x4,%ebp
	if (block_mac < block_max) {
		/* grow the block array */
		tommy_array_grow(&array->block, block_max);

		/* allocate new blocks */
		while (block_mac < block_max) {
     d78:	39 fb                	cmp    %edi,%ebx
 * You must be sure that space for this position is already
 * allocated calling tommy_array_grow().
 */
tommy_inline void tommy_array_set(tommy_array* array, unsigned pos, void* element)
{
	*tommy_array_ref(array, pos) = element;
     d7a:	89 30                	mov    %esi,(%eax)
     d7c:	75 c1                	jne    d3f <tommy_arrayblk_grow+0x10f>
     d7e:	8b 74 24 30          	mov    0x30(%esp),%esi
     d82:	e9 cb fe ff ff       	jmp    c52 <tommy_arrayblk_grow+0x22>
     d87:	e8 fc ff ff ff       	call   d88 <tommy_arrayblk_grow+0x158>
     d8c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00000d90 <tommy_arrayblk_memory_usage>:
	if (array->size < size)
		array->size = size;
}

tommy_size_t tommy_arrayblk_memory_usage(tommy_arrayblk* array)
{
     d90:	8b 54 24 04          	mov    0x4(%esp),%edx
	return tommy_array_memory_usage(&array->block) + tommy_array_size(&array->block) * TOMMY_ARRAYBLK_SIZE * sizeof(void*);
     d94:	8b 82 8c 00 00 00    	mov    0x8c(%edx),%eax
}

tommy_size_t tommy_array_memory_usage(tommy_array* array)
{
	return array->bucket_max * (tommy_size_t)sizeof(void*);
     d9a:	8b 92 84 00 00 00    	mov    0x84(%edx),%edx
     da0:	c1 e0 12             	shl    $0x12,%eax
     da3:	8d 04 90             	lea    (%eax,%edx,4),%eax
}
     da6:	c3                   	ret    
     da7:	89 f6                	mov    %esi,%esi
     da9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000db0 <tommy_arrayblkof_init>:

/******************************************************************************/
/* array */

void tommy_arrayblkof_init(tommy_arrayblkof* array, unsigned element_size)
{
     db0:	57                   	push   %edi
     db1:	53                   	push   %ebx
     db2:	83 ec 10             	sub    $0x10,%esp
     db5:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
/* array */

void tommy_array_init(tommy_array* array)
{
	/* fixed initial size */
	array->bucket_bit = TOMMY_ARRAY_BIT;
     db9:	c7 83 80 00 00 00 06 	movl   $0x6,0x80(%ebx)
     dc0:	00 00 00 
	array->bucket_max = 1 << array->bucket_bit;
     dc3:	c7 83 84 00 00 00 40 	movl   $0x40,0x84(%ebx)
     dca:	00 00 00 
	array->bucket[0] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     dcd:	68 00 01 00 00       	push   $0x100
     dd2:	e8 fc ff ff ff       	call   dd3 <tommy_arrayblkof_init+0x23>
     dd7:	89 c2                	mov    %eax,%edx
     dd9:	89 03                	mov    %eax,(%ebx)

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * sizeof(void*));
     ddb:	b9 40 00 00 00       	mov    $0x40,%ecx
     de0:	31 c0                	xor    %eax,%eax
     de2:	89 d7                	mov    %edx,%edi

	array->bucket_mac = 1;
     de4:	c7 83 88 00 00 00 01 	movl   $0x1,0x88(%ebx)
     deb:	00 00 00 
	array->bucket_bit = TOMMY_ARRAY_BIT;
	array->bucket_max = 1 << array->bucket_bit;
	array->bucket[0] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));

	/* initializes it with zeros */
	memset(array->bucket[0], 0, array->bucket_max * sizeof(void*));
     dee:	f3 ab                	rep stos %eax,%es:(%edi)
	tommy_array_init(&array->block);
	array->element_size = element_size;
     df0:	8b 44 24 24          	mov    0x24(%esp),%eax

	array->bucket_mac = 1;
	array->size = 0;
     df4:	c7 83 8c 00 00 00 00 	movl   $0x0,0x8c(%ebx)
     dfb:	00 00 00 
	array->size = 0;
     dfe:	c7 83 94 00 00 00 00 	movl   $0x0,0x94(%ebx)
     e05:	00 00 00 
/* array */

void tommy_arrayblkof_init(tommy_arrayblkof* array, unsigned element_size)
{
	tommy_array_init(&array->block);
	array->element_size = element_size;
     e08:	89 83 90 00 00 00    	mov    %eax,0x90(%ebx)
	array->size = 0;
}
     e0e:	83 c4 14             	add    $0x14,%esp
     e11:	5b                   	pop    %ebx
     e12:	5f                   	pop    %edi
     e13:	c3                   	ret    
     e14:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     e1a:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

00000e20 <tommy_arrayblkof_done>:

void tommy_arrayblkof_done(tommy_arrayblkof* array)
{
     e20:	57                   	push   %edi
     e21:	56                   	push   %esi

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
     e22:	bf 01 00 00 00       	mov    $0x1,%edi
     e27:	53                   	push   %ebx
     e28:	8b 74 24 10          	mov    0x10(%esp),%esi
	unsigned i;

	for(i=0;i<tommy_array_size(&array->block);++i)
     e2c:	31 db                	xor    %ebx,%ebx
     e2e:	8b 96 8c 00 00 00    	mov    0x8c(%esi),%edx
     e34:	85 d2                	test   %edx,%edx
     e36:	75 32                	jne    e6a <tommy_arrayblkof_done+0x4a>
     e38:	eb 46                	jmp    e80 <tommy_arrayblkof_done+0x60>
     e3a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
     e40:	0f bd cb             	bsr    %ebx,%ecx
     e43:	89 f8                	mov    %edi,%eax
     e45:	89 da                	mov    %ebx,%edx
     e47:	d3 e0                	shl    %cl,%eax
     e49:	29 c2                	sub    %eax,%edx

	return &array->bucket[bsr - TOMMY_ARRAY_BIT + 1][pos];
     e4b:	8b 44 8e ec          	mov    -0x14(%esi,%ecx,4),%eax
     e4f:	8d 04 90             	lea    (%eax,%edx,4),%eax
		tommy_free(tommy_array_get(&array->block, i));
     e52:	83 ec 0c             	sub    $0xc,%esp
     e55:	ff 30                	pushl  (%eax)

void tommy_arrayblkof_done(tommy_arrayblkof* array)
{
	unsigned i;

	for(i=0;i<tommy_array_size(&array->block);++i)
     e57:	83 c3 01             	add    $0x1,%ebx
		tommy_free(tommy_array_get(&array->block, i));
     e5a:	e8 fc ff ff ff       	call   e5b <tommy_arrayblkof_done+0x3b>

void tommy_arrayblkof_done(tommy_arrayblkof* array)
{
	unsigned i;

	for(i=0;i<tommy_array_size(&array->block);++i)
     e5f:	83 c4 10             	add    $0x10,%esp
     e62:	39 9e 8c 00 00 00    	cmp    %ebx,0x8c(%esi)
     e68:	76 16                	jbe    e80 <tommy_arrayblkof_done+0x60>
	unsigned bsr;  

	assert(pos < array->size);

	/* special case for the first bucket */
	if (pos < (1 << TOMMY_ARRAY_BIT)) {
     e6a:	83 fb 3f             	cmp    $0x3f,%ebx
     e6d:	77 d1                	ja     e40 <tommy_arrayblkof_done+0x20>
		return &array->bucket[0][pos];
     e6f:	8b 06                	mov    (%esi),%eax
     e71:	8d 04 98             	lea    (%eax,%ebx,4),%eax
     e74:	eb dc                	jmp    e52 <tommy_arrayblkof_done+0x32>
     e76:	8d 76 00             	lea    0x0(%esi),%esi
     e79:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     e80:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
     e86:	31 db                	xor    %ebx,%ebx
     e88:	85 c0                	test   %eax,%eax
     e8a:	74 1d                	je     ea9 <tommy_arrayblkof_done+0x89>
     e8c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
		tommy_free(array->bucket[i]);
     e90:	83 ec 0c             	sub    $0xc,%esp
     e93:	ff 34 9e             	pushl  (%esi,%ebx,4)
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     e96:	83 c3 01             	add    $0x1,%ebx
		tommy_free(array->bucket[i]);
     e99:	e8 fc ff ff ff       	call   e9a <tommy_arrayblkof_done+0x7a>
}

void tommy_array_done(tommy_array* array)
{
	unsigned i;
	for(i=0;i<array->bucket_mac;++i)
     e9e:	83 c4 10             	add    $0x10,%esp
     ea1:	3b 9e 88 00 00 00    	cmp    0x88(%esi),%ebx
     ea7:	72 e7                	jb     e90 <tommy_arrayblkof_done+0x70>
		tommy_free(tommy_array_get(&array->block, i));

	tommy_array_done(&array->block);
}
     ea9:	5b                   	pop    %ebx
     eaa:	5e                   	pop    %esi
     eab:	5f                   	pop    %edi
     eac:	c3                   	ret    
     ead:	8d 76 00             	lea    0x0(%esi),%esi

00000eb0 <tommy_arrayblkof_grow>:

void tommy_arrayblkof_grow(tommy_arrayblkof* array, unsigned size)
{
     eb0:	55                   	push   %ebp
     eb1:	57                   	push   %edi
     eb2:	56                   	push   %esi
     eb3:	53                   	push   %ebx
     eb4:	83 ec 1c             	sub    $0x1c,%esp
     eb7:	8b 74 24 30          	mov    0x30(%esp),%esi
	unsigned block_max = (size + TOMMY_ARRAYBLK_SIZE - 1) / TOMMY_ARRAYBLK_SIZE;
     ebb:	8b 44 24 34          	mov    0x34(%esp),%eax
     ebf:	8b 9e 8c 00 00 00    	mov    0x8c(%esi),%ebx
     ec5:	05 ff ff 00 00       	add    $0xffff,%eax
     eca:	c1 e8 10             	shr    $0x10,%eax
     ecd:	89 44 24 04          	mov    %eax,0x4(%esp)
	unsigned block_mac = tommy_array_size(&array->block);

	if (block_mac < block_max) {
     ed1:	39 d8                	cmp    %ebx,%eax
     ed3:	77 23                	ja     ef8 <tommy_arrayblkof_grow+0x48>

			++block_mac;
		}
	}

	if (array->size < size)
     ed5:	8b 86 94 00 00 00    	mov    0x94(%esi),%eax
     edb:	39 44 24 34          	cmp    %eax,0x34(%esp)
     edf:	76 0a                	jbe    eeb <tommy_arrayblkof_grow+0x3b>
		array->size = size;
     ee1:	8b 44 24 34          	mov    0x34(%esp),%eax
     ee5:	89 86 94 00 00 00    	mov    %eax,0x94(%esi)
}
     eeb:	83 c4 1c             	add    $0x1c,%esp
     eee:	5b                   	pop    %ebx
     eef:	5e                   	pop    %esi
     ef0:	5f                   	pop    %edi
     ef1:	5d                   	pop    %ebp
     ef2:	c3                   	ret    
     ef3:	90                   	nop
     ef4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     ef8:	8b ae 84 00 00 00    	mov    0x84(%esi),%ebp
     efe:	39 6c 24 04          	cmp    %ebp,0x4(%esp)
     f02:	76 7b                	jbe    f7f <tommy_arrayblkof_grow+0xcf>
     f04:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
     f0a:	8b 8e 80 00 00 00    	mov    0x80(%esi),%ecx
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     f10:	89 5c 24 0c          	mov    %ebx,0xc(%esp)
     f14:	89 f3                	mov    %esi,%ebx
     f16:	89 c2                	mov    %eax,%edx
     f18:	83 c0 01             	add    $0x1,%eax
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     f1b:	89 cf                	mov    %ecx,%edi
     f1d:	29 ca                	sub    %ecx,%edx
     f1f:	29 c8                	sub    %ecx,%eax
     f21:	8d 14 96             	lea    (%esi,%edx,4),%edx
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     f24:	89 c6                	mov    %eax,%esi
     f26:	89 54 24 08          	mov    %edx,0x8(%esp)
     f2a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     f30:	83 ec 0c             	sub    $0xc,%esp
     f33:	c1 e5 02             	shl    $0x2,%ebp
     f36:	55                   	push   %ebp
     f37:	e8 fc ff ff ff       	call   f38 <tommy_arrayblkof_grow+0x88>
     f3c:	8b 54 24 18          	mov    0x18(%esp),%edx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     f40:	83 c4 0c             	add    $0xc,%esp

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));
     f43:	89 04 ba             	mov    %eax,(%edx,%edi,4)

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     f46:	55                   	push   %ebp

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     f47:	bd 01 00 00 00       	mov    $0x1,%ebp
	while (size > array->bucket_max) {
		/* allocate one more bucket */
		array->bucket[array->bucket_mac] = tommy_cast(void**, tommy_malloc(array->bucket_max * sizeof(void*)));

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));
     f4c:	6a 00                	push   $0x0
     f4e:	50                   	push   %eax
     f4f:	e8 fc ff ff ff       	call   f50 <tommy_arrayblkof_grow+0xa0>
     f54:	8d 04 3e             	lea    (%esi,%edi,1),%eax

		++array->bucket_mac;
		++array->bucket_bit;
     f57:	83 c7 01             	add    $0x1,%edi
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     f5a:	83 c4 10             	add    $0x10,%esp
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     f5d:	89 f9                	mov    %edi,%ecx

		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
     f5f:	89 bb 80 00 00 00    	mov    %edi,0x80(%ebx)
		array->bucket_max = 1 << array->bucket_bit;
     f65:	d3 e5                	shl    %cl,%ebp
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     f67:	39 6c 24 04          	cmp    %ebp,0x4(%esp)
     f6b:	89 83 88 00 00 00    	mov    %eax,0x88(%ebx)
		/* initializes it with zeros */
		memset(array->bucket[array->bucket_mac], 0, array->bucket_max * sizeof(void*));

		++array->bucket_mac;
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
     f71:	89 ab 84 00 00 00    	mov    %ebp,0x84(%ebx)
		tommy_free(array->bucket[i]);
}

void tommy_array_grow(tommy_array* array, unsigned size)
{
	while (size > array->bucket_max) {
     f77:	77 b7                	ja     f30 <tommy_arrayblkof_grow+0x80>
     f79:	89 de                	mov    %ebx,%esi
     f7b:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
		++array->bucket_bit;
		array->bucket_max = 1 << array->bucket_bit;
	}

	if (array->size < size)
		array->size = size;
     f7f:	8b 44 24 04          	mov    0x4(%esp),%eax
     f83:	8b ae 90 00 00 00    	mov    0x90(%esi),%ebp
     f89:	89 74 24 30          	mov    %esi,0x30(%esp)
     f8d:	89 86 8c 00 00 00    	mov    %eax,0x8c(%esi)
     f93:	8d 04 9d 00 00 00 00 	lea    0x0(,%ebx,4),%eax
     f9a:	c1 e5 10             	shl    $0x10,%ebp
     f9d:	89 c7                	mov    %eax,%edi
     f9f:	eb 30                	jmp    fd1 <tommy_arrayblkof_grow+0x121>
     fa1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
     fa8:	0f bd cb             	bsr    %ebx,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
     fab:	b8 01 00 00 00       	mov    $0x1,%eax
     fb0:	89 da                	mov    %ebx,%edx
     fb2:	d3 e0                	shl    %cl,%eax
			memset(ptr, 0, TOMMY_ARRAYBLK_SIZE * array->element_size);

			/* set the new block */
			tommy_array_set(&array->block, block_mac, ptr);

			++block_mac;
     fb4:	83 c3 01             	add    $0x1,%ebx
     fb7:	83 c7 04             	add    $0x4,%edi
     fba:	29 c2                	sub    %eax,%edx
	if (block_mac < block_max) {
		/* grow the block array */
		tommy_array_grow(&array->block, block_max);

		/* allocate new blocks */
		while (block_mac < block_max) {
     fbc:	3b 5c 24 04          	cmp    0x4(%esp),%ebx
     fc0:	89 d0                	mov    %edx,%eax

	return &array->bucket[bsr - TOMMY_ARRAY_BIT + 1][pos];
     fc2:	8b 54 24 30          	mov    0x30(%esp),%edx
     fc6:	8b 4c 8a ec          	mov    -0x14(%edx,%ecx,4),%ecx
     fca:	8d 04 81             	lea    (%ecx,%eax,4),%eax
 * You must be sure that space for this position is already
 * allocated calling tommy_array_grow().
 */
tommy_inline void tommy_array_set(tommy_array* array, unsigned pos, void* element)
{
	*tommy_array_ref(array, pos) = element;
     fcd:	89 30                	mov    %esi,(%eax)
     fcf:	74 3b                	je     100c <tommy_arrayblkof_grow+0x15c>
			void* ptr = tommy_malloc(TOMMY_ARRAYBLK_SIZE * array->element_size);
     fd1:	83 ec 0c             	sub    $0xc,%esp
     fd4:	55                   	push   %ebp
     fd5:	e8 fc ff ff ff       	call   fd6 <tommy_arrayblkof_grow+0x126>

			/* initializes it with zeros */
			memset(ptr, 0, TOMMY_ARRAYBLK_SIZE * array->element_size);
     fda:	83 c4 0c             	add    $0xc,%esp
		/* grow the block array */
		tommy_array_grow(&array->block, block_max);

		/* allocate new blocks */
		while (block_mac < block_max) {
			void* ptr = tommy_malloc(TOMMY_ARRAYBLK_SIZE * array->element_size);
     fdd:	89 c6                	mov    %eax,%esi

			/* initializes it with zeros */
			memset(ptr, 0, TOMMY_ARRAYBLK_SIZE * array->element_size);
     fdf:	55                   	push   %ebp
     fe0:	6a 00                	push   $0x0
     fe2:	50                   	push   %eax
     fe3:	e8 fc ff ff ff       	call   fe4 <tommy_arrayblkof_grow+0x134>
 */
tommy_inline void** tommy_array_ref(tommy_array* array, unsigned pos)
{
	unsigned bsr;  

	assert(pos < array->size);
     fe8:	83 c4 10             	add    $0x10,%esp
     feb:	3b 5c 24 04          	cmp    0x4(%esp),%ebx
     fef:	74 24                	je     1015 <tommy_arrayblkof_grow+0x165>

	/* special case for the first bucket */
	if (pos < (1 << TOMMY_ARRAY_BIT)) {
     ff1:	83 fb 3f             	cmp    $0x3f,%ebx
     ff4:	77 b2                	ja     fa8 <tommy_arrayblkof_grow+0xf8>
		return &array->bucket[0][pos];
     ff6:	8b 54 24 30          	mov    0x30(%esp),%edx

			/* set the new block */
			tommy_array_set(&array->block, block_mac, ptr);

			++block_mac;
     ffa:	83 c3 01             	add    $0x1,%ebx
     ffd:	8b 02                	mov    (%edx),%eax
     fff:	01 f8                	add    %edi,%eax
    1001:	83 c7 04             	add    $0x4,%edi
	if (block_mac < block_max) {
		/* grow the block array */
		tommy_array_grow(&array->block, block_max);

		/* allocate new blocks */
		while (block_mac < block_max) {
    1004:	3b 5c 24 04          	cmp    0x4(%esp),%ebx
 * You must be sure that space for this position is already
 * allocated calling tommy_array_grow().
 */
tommy_inline void tommy_array_set(tommy_array* array, unsigned pos, void* element)
{
	*tommy_array_ref(array, pos) = element;
    1008:	89 30                	mov    %esi,(%eax)
    100a:	75 c5                	jne    fd1 <tommy_arrayblkof_grow+0x121>
    100c:	8b 74 24 30          	mov    0x30(%esp),%esi
    1010:	e9 c0 fe ff ff       	jmp    ed5 <tommy_arrayblkof_grow+0x25>
    1015:	e8 fc ff ff ff       	call   1016 <tommy_arrayblkof_grow+0x166>
    101a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

00001020 <tommy_arrayblkof_memory_usage>:
	if (array->size < size)
		array->size = size;
}

tommy_size_t tommy_arrayblkof_memory_usage(tommy_arrayblkof* array)
{
    1020:	8b 54 24 04          	mov    0x4(%esp),%edx
    1024:	8b 82 90 00 00 00    	mov    0x90(%edx),%eax
    102a:	c1 e0 10             	shl    $0x10,%eax
	return tommy_array_memory_usage(&array->block) + tommy_array_size(&array->block) * TOMMY_ARRAYBLKOF_SIZE * array->element_size;
    102d:	0f af 82 8c 00 00 00 	imul   0x8c(%edx),%eax
}

tommy_size_t tommy_array_memory_usage(tommy_array* array)
{
	return array->bucket_max * (tommy_size_t)sizeof(void*);
    1034:	8b 92 84 00 00 00    	mov    0x84(%edx),%edx
    103a:	8d 04 90             	lea    (%eax,%edx,4),%eax
}
    103d:	c3                   	ret    
    103e:	66 90                	xchg   %ax,%ax

00001040 <tommy_list_concat>:

#include "tommylist.h"
#include "tommychain.h"

void tommy_list_concat(tommy_list* first, tommy_list* second)
{
    1040:	53                   	push   %ebx
    1041:	8b 44 24 0c          	mov    0xc(%esp),%eax
    1045:	8b 00                	mov    (%eax),%eax
	tommy_node* first_head;
	tommy_node* first_tail;
	tommy_node* second_head;

	if (tommy_list_empty(second)) {
    1047:	85 c0                	test   %eax,%eax
    1049:	74 18                	je     1063 <tommy_list_concat+0x23>
    104b:	8b 4c 24 08          	mov    0x8(%esp),%ecx
    104f:	8b 11                	mov    (%ecx),%edx
		return;
	}

	if (tommy_list_empty(first)) {
    1051:	85 d2                	test   %edx,%edx
    1053:	74 13                	je     1068 <tommy_list_concat+0x28>
	tommy_node* head = tommy_list_head(list);

	if (!head)
		return 0;

	return head->prev;
    1055:	8b 4a 04             	mov    0x4(%edx),%ecx
	first_head = tommy_list_head(first);
	second_head = tommy_list_head(second);
	first_tail = tommy_list_tail(first);

	/* set the "circular" prev list */
	first_head->prev = second_head->prev;
    1058:	8b 58 04             	mov    0x4(%eax),%ebx
    105b:	89 5a 04             	mov    %ebx,0x4(%edx)
	second_head->prev = first_tail;
    105e:	89 48 04             	mov    %ecx,0x4(%eax)

	/* set the "0 terminated" next list */
	first_tail->next = second_head;
    1061:	89 01                	mov    %eax,(%ecx)
}
    1063:	5b                   	pop    %ebx
    1064:	c3                   	ret    
    1065:	8d 76 00             	lea    0x0(%esi),%esi
	if (tommy_list_empty(second)) {
		return;
	}

	if (tommy_list_empty(first)) {
		*first = *second;
    1068:	89 01                	mov    %eax,(%ecx)
	first_head->prev = second_head->prev;
	second_head->prev = first_tail;

	/* set the "0 terminated" next list */
	first_tail->next = second_head;
}
    106a:	5b                   	pop    %ebx
    106b:	c3                   	ret    
    106c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00001070 <tommy_list_sort>:
	tail->next = 0;
	*list = head;
}

void tommy_list_sort(tommy_list* list, tommy_compare_func* cmp)
{
    1070:	53                   	push   %ebx
    1071:	83 ec 18             	sub    $0x18,%esp
    1074:	8b 5c 24 20          	mov    0x20(%esp),%ebx
    1078:	8b 03                	mov    (%ebx),%eax
	tommy_chain chain;
	tommy_node* head;

	if (tommy_list_empty(list))
    107a:	85 c0                	test   %eax,%eax
    107c:	74 2b                	je     10a9 <tommy_list_sort+0x39>
		return;

	head = tommy_list_head(list);

	/* create a chain from the list */
	chain.head = head;
    107e:	89 44 24 08          	mov    %eax,0x8(%esp)
	chain.tail = head->prev;
    1082:	8b 40 04             	mov    0x4(%eax),%eax

	tommy_chain_mergesort(&chain, cmp);
    1085:	8b 54 24 24          	mov    0x24(%esp),%edx

	head = tommy_list_head(list);

	/* create a chain from the list */
	chain.head = head;
	chain.tail = head->prev;
    1089:	89 44 24 0c          	mov    %eax,0xc(%esp)

	tommy_chain_mergesort(&chain, cmp);
    108d:	8d 44 24 08          	lea    0x8(%esp),%eax
    1091:	e8 ea f0 ff ff       	call   180 <tommy_chain_mergesort>

	/* restore the list */
	tommy_list_set(list, chain.head, chain.tail);
    1096:	8b 44 24 08          	mov    0x8(%esp),%eax
    109a:	8b 54 24 0c          	mov    0xc(%esp),%edx
/** \internal
 * Setup a list.
 */
tommy_inline void tommy_list_set(tommy_list* list, tommy_node* head, tommy_node* tail)
{
	head->prev = tail;
    109e:	89 50 04             	mov    %edx,0x4(%eax)
	tail->next = 0;
    10a1:	c7 02 00 00 00 00    	movl   $0x0,(%edx)
	*list = head;
    10a7:	89 03                	mov    %eax,(%ebx)

	tommy_chain_mergesort(&chain, cmp);

	/* restore the list */
	tommy_list_set(list, chain.head, chain.tail);
}
    10a9:	83 c4 18             	add    $0x18,%esp
    10ac:	5b                   	pop    %ebx
    10ad:	c3                   	ret    
    10ae:	66 90                	xchg   %ax,%ax

000010b0 <tommy_trie_init>:
#define trie_get_type(ptr) (((tommy_uintptr_t)(ptr)) & 1)
#define trie_get_tree(ptr) ((tommy_trie_tree*)(((tommy_uintptr_t)(ptr)) - TOMMY_TRIE_TYPE_TREE))
#define trie_set_tree(ptr) (void*)(((tommy_uintptr_t)(ptr)) + TOMMY_TRIE_TYPE_TREE)

void tommy_trie_init(tommy_trie* trie, tommy_allocator* alloc)
{
    10b0:	57                   	push   %edi
    10b1:	8b 54 24 08          	mov    0x8(%esp),%edx
    10b5:	31 c0                	xor    %eax,%eax
    10b7:	8d 7a 04             	lea    0x4(%edx),%edi
    10ba:	89 d1                	mov    %edx,%ecx
    10bc:	c7 02 00 00 00 00    	movl   $0x0,(%edx)
    10c2:	c7 42 3c 00 00 00 00 	movl   $0x0,0x3c(%edx)
    10c9:	83 e7 fc             	and    $0xfffffffc,%edi
    10cc:	29 f9                	sub    %edi,%ecx
    10ce:	83 c1 40             	add    $0x40,%ecx
    10d1:	c1 e9 02             	shr    $0x2,%ecx
    10d4:	f3 ab                	rep stos %eax,%es:(%edi)
	unsigned i;

	for(i=0;i<TOMMY_TRIE_BUCKET_MAX;++i)
		trie->bucket[i] = 0;
   
	trie->count = 0;
    10d6:	c7 42 40 00 00 00 00 	movl   $0x0,0x40(%edx)
	trie->node_count = 0;
    10dd:	c7 42 44 00 00 00 00 	movl   $0x0,0x44(%edx)

	trie->alloc = alloc;
    10e4:	8b 44 24 0c          	mov    0xc(%esp),%eax
    10e8:	89 42 48             	mov    %eax,0x48(%edx)
}
    10eb:	5f                   	pop    %edi
    10ec:	c3                   	ret    
    10ed:	8d 76 00             	lea    0x0(%esi),%esi

000010f0 <tommy_trie_insert>:
	shift -= TOMMY_TRIE_TREE_BIT;
	goto expand;
}

void tommy_trie_insert(tommy_trie* trie, tommy_trie_node* node, void* data, tommy_key_t key)
{
    10f0:	55                   	push   %ebp
    10f1:	57                   	push   %edi
    10f2:	56                   	push   %esi
    10f3:	53                   	push   %ebx
    10f4:	83 ec 2c             	sub    $0x2c,%esp
	tommy_trie_node** let_ptr;

	node->data = data;
    10f7:	8b 7c 24 44          	mov    0x44(%esp),%edi
    10fb:	8b 44 24 48          	mov    0x48(%esp),%eax
	node->key = key;

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    10ff:	8b 74 24 40          	mov    0x40(%esp),%esi

void tommy_trie_insert(tommy_trie* trie, tommy_trie_node* node, void* data, tommy_key_t key)
{
	tommy_trie_node** let_ptr;

	node->data = data;
    1103:	89 47 08             	mov    %eax,0x8(%edi)
	node->key = key;
    1106:	89 f8                	mov    %edi,%eax
    1108:	8b 7c 24 4c          	mov    0x4c(%esp),%edi
    110c:	89 78 0c             	mov    %edi,0xc(%eax)

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    110f:	89 f8                	mov    %edi,%eax
    1111:	c1 e8 1c             	shr    $0x1c,%eax
    1114:	8b 2c 86             	mov    (%esi,%eax,4),%ebp
    1117:	8d 3c 86             	lea    (%esi,%eax,4),%edi

recurse:
	ptr = *let_ptr;

	/* if null, just insert the node */
	if (!ptr) {
    111a:	85 ed                	test   %ebp,%ebp
    111c:	74 3b                	je     1159 <tommy_trie_insert+0x69>
		/* setup the node as a list */
		tommy_list_insert_first(let_ptr, insert);
		return;
	}

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    111e:	f7 c5 01 00 00 00    	test   $0x1,%ebp
    1124:	b9 1c 00 00 00       	mov    $0x1c,%ecx
    1129:	8b 54 24 4c          	mov    0x4c(%esp),%edx
    112d:	75 11                	jne    1140 <tommy_trie_insert+0x50>
    112f:	eb 4f                	jmp    1180 <tommy_trie_insert+0x90>
    1131:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    1138:	f7 c5 01 00 00 00    	test   $0x1,%ebp
    113e:	74 40                	je     1180 <tommy_trie_insert+0x90>
		/* repeat the process one level down */
		let_ptr = &trie_get_tree(ptr)->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1140:	89 d0                	mov    %edx,%eax
    1142:	83 ed 01             	sub    $0x1,%ebp
    1145:	d3 e8                	shr    %cl,%eax
		shift -= TOMMY_TRIE_TREE_BIT;
    1147:	83 e9 04             	sub    $0x4,%ecx
		return;
	}

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		/* repeat the process one level down */
		let_ptr = &trie_get_tree(ptr)->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    114a:	83 e0 0f             	and    $0xf,%eax
    114d:	8d 7c 85 00          	lea    0x0(%ebp,%eax,4),%edi
    1151:	8b 6c 85 00          	mov    0x0(%ebp,%eax,4),%ebp

recurse:
	ptr = *let_ptr;

	/* if null, just insert the node */
	if (!ptr) {
    1155:	85 ed                	test   %ebp,%ebp
    1157:	75 df                	jne    1138 <tommy_trie_insert+0x48>
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    1159:	8b 44 24 44          	mov    0x44(%esp),%eax
    115d:	89 40 04             	mov    %eax,0x4(%eax)

	/* one element "0 terminated" next list */
	node->next = 0;
    1160:	c7 00 00 00 00 00    	movl   $0x0,(%eax)

	*list = node;
    1166:	89 07                	mov    %eax,(%edi)

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];

	trie_bucket_insert(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, node, key);

	++trie->count;
    1168:	8b 44 24 40          	mov    0x40(%esp),%eax
    116c:	83 40 40 01          	addl   $0x1,0x40(%eax)
}
    1170:	83 c4 2c             	add    $0x2c,%esp
    1173:	5b                   	pop    %ebx
    1174:	5e                   	pop    %esi
    1175:	5f                   	pop    %edi
    1176:	5d                   	pop    %ebp
    1177:	c3                   	ret    
    1178:	90                   	nop
    1179:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
	}

	node = tommy_cast(tommy_trie_node*, ptr);

	/* if it's the same key, insert in the list */
	if (node->key == key) {
    1180:	8b 45 0c             	mov    0xc(%ebp),%eax
    1183:	39 44 24 4c          	cmp    %eax,0x4c(%esp)
    1187:	0f 84 82 01 00 00    	je     130f <tommy_trie_insert+0x21f>
    118d:	89 6c 24 10          	mov    %ebp,0x10(%esp)
    1191:	89 7c 24 0c          	mov    %edi,0xc(%esp)
    1195:	8b 6c 24 40          	mov    0x40(%esp),%ebp
    1199:	e9 b1 00 00 00       	jmp    124f <tommy_trie_insert+0x15f>
    119e:	66 90                	xchg   %ax,%ax
    11a0:	8b 03                	mov    (%ebx),%eax
		}
	}

	/* remove one from the free list */
	ptr = alloc->free_block;
	alloc->free_block = alloc->free_block->next;
    11a2:	89 06                	mov    %eax,(%esi)
	
	++alloc->count;
    11a4:	83 46 10 01          	addl   $0x1,0x10(%esi)

expand:
	/* convert to a tree */
	tree = tommy_cast(tommy_trie_tree*, tommy_allocator_alloc(trie->alloc));
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));
    11a8:	8d 43 01             	lea    0x1(%ebx),%eax
    11ab:	8b 74 24 0c          	mov    0xc(%esp),%esi
	}

expand:
	/* convert to a tree */
	tree = tommy_cast(tommy_trie_tree*, tommy_allocator_alloc(trie->alloc));
	++trie->node_count;
    11af:	83 45 44 01          	addl   $0x1,0x44(%ebp)
		tree->map[i] = 0;
	}

	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
	j = (key >> shift) & TOMMY_TRIE_TREE_MASK;
    11b3:	8b 54 24 4c          	mov    0x4c(%esp),%edx

expand:
	/* convert to a tree */
	tree = tommy_cast(tommy_trie_tree*, tommy_allocator_alloc(trie->alloc));
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));
    11b7:	89 06                	mov    %eax,(%esi)
		tree->map[i] = 0;
	}

	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
	j = (key >> shift) & TOMMY_TRIE_TREE_MASK;
    11b9:	d3 ea                	shr    %cl,%edx
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		tree->map[i] = 0;
	}

	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
    11bb:	8b 44 24 10          	mov    0x10(%esp),%eax
	j = (key >> shift) & TOMMY_TRIE_TREE_MASK;
    11bf:	83 e2 0f             	and    $0xf,%edx
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));

	/* initialize it */
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		tree->map[i] = 0;
    11c2:	c7 03 00 00 00 00    	movl   $0x0,(%ebx)
    11c8:	c7 43 04 00 00 00 00 	movl   $0x0,0x4(%ebx)
    11cf:	c7 43 08 00 00 00 00 	movl   $0x0,0x8(%ebx)
    11d6:	c7 43 0c 00 00 00 00 	movl   $0x0,0xc(%ebx)
    11dd:	c7 43 10 00 00 00 00 	movl   $0x0,0x10(%ebx)
	}

	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
    11e4:	8b 40 0c             	mov    0xc(%eax),%eax
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));

	/* initialize it */
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		tree->map[i] = 0;
    11e7:	c7 43 14 00 00 00 00 	movl   $0x0,0x14(%ebx)
    11ee:	c7 43 18 00 00 00 00 	movl   $0x0,0x18(%ebx)
    11f5:	c7 43 1c 00 00 00 00 	movl   $0x0,0x1c(%ebx)
    11fc:	c7 43 20 00 00 00 00 	movl   $0x0,0x20(%ebx)
    1203:	c7 43 24 00 00 00 00 	movl   $0x0,0x24(%ebx)
	}

	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
    120a:	89 44 24 0c          	mov    %eax,0xc(%esp)
    120e:	d3 e8                	shr    %cl,%eax
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));

	/* initialize it */
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		tree->map[i] = 0;
    1210:	c7 43 28 00 00 00 00 	movl   $0x0,0x28(%ebx)
	}

	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
    1217:	83 e0 0f             	and    $0xf,%eax
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));

	/* initialize it */
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		tree->map[i] = 0;
    121a:	c7 43 2c 00 00 00 00 	movl   $0x0,0x2c(%ebx)
    1221:	c7 43 30 00 00 00 00 	movl   $0x0,0x30(%ebx)
	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
	j = (key >> shift) & TOMMY_TRIE_TREE_MASK;

	/* if they don't collide */
	if (i != j) {
    1228:	39 d0                	cmp    %edx,%eax
	++trie->node_count;
	*let_ptr = tommy_cast(tommy_trie_node*, trie_set_tree(tree));

	/* initialize it */
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		tree->map[i] = 0;
    122a:	c7 43 34 00 00 00 00 	movl   $0x0,0x34(%ebx)
    1231:	c7 43 38 00 00 00 00 	movl   $0x0,0x38(%ebx)
    1238:	c7 43 3c 00 00 00 00 	movl   $0x0,0x3c(%ebx)
	/* get the position of the two elements */
	i = (node->key >> shift) & TOMMY_TRIE_TREE_MASK;
	j = (key >> shift) & TOMMY_TRIE_TREE_MASK;

	/* if they don't collide */
	if (i != j) {
    123f:	0f 85 a3 00 00 00    	jne    12e8 <tommy_trie_insert+0x1f8>
		tommy_list_insert_first(&tree->map[j], insert);
		return;
	}

	/* expand one more level */
	let_ptr = &tree->map[i];
    1245:	8d 04 83             	lea    (%ebx,%eax,4),%eax
	shift -= TOMMY_TRIE_TREE_BIT;
    1248:	83 e9 04             	sub    $0x4,%ecx
		tommy_list_insert_first(&tree->map[j], insert);
		return;
	}

	/* expand one more level */
	let_ptr = &tree->map[i];
    124b:	89 44 24 0c          	mov    %eax,0xc(%esp)
		return;
	}

expand:
	/* convert to a tree */
	tree = tommy_cast(tommy_trie_tree*, tommy_allocator_alloc(trie->alloc));
    124f:	8b 75 48             	mov    0x48(%ebp),%esi
void* tommy_allocator_alloc(tommy_allocator* alloc)
{
	void* ptr;

	/* if no free block available */
	if (!alloc->free_block) {
    1252:	8b 1e                	mov    (%esi),%ebx
    1254:	85 db                	test   %ebx,%ebx
    1256:	0f 85 44 ff ff ff    	jne    11a0 <tommy_trie_insert+0xb0>
    125c:	89 4c 24 1c          	mov    %ecx,0x1c(%esp)
		tommy_uintptr_t off, mis;
		tommy_size_t size = TOMMY_ALLOCATOR_BLOCK_SIZE;
		char* data = tommy_cast(char*, tommy_malloc(size));
    1260:	83 ec 0c             	sub    $0xc,%esp
    1263:	68 c0 0f 00 00       	push   $0xfc0
    1268:	e8 fc ff ff ff       	call   1269 <tommy_trie_insert+0x179>
		tommy_allocator_entry* segment = (tommy_allocator_entry*)data;

		/* put in the segment list */
		segment->next = alloc->used_segment;
    126d:	8b 56 04             	mov    0x4(%esi),%edx
		alloc->used_segment = segment;
		data += sizeof(tommy_allocator_entry);
    1270:	8d 78 04             	lea    0x4(%eax),%edi
		tommy_size_t size = TOMMY_ALLOCATOR_BLOCK_SIZE;
		char* data = tommy_cast(char*, tommy_malloc(size));
		tommy_allocator_entry* segment = (tommy_allocator_entry*)data;

		/* put in the segment list */
		segment->next = alloc->used_segment;
    1273:	89 10                	mov    %edx,(%eax)
		alloc->used_segment = segment;
    1275:	89 46 04             	mov    %eax,0x4(%esi)
		data += sizeof(tommy_allocator_entry);

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
    1278:	31 d2                	xor    %edx,%edx
    127a:	8b 46 0c             	mov    0xc(%esi),%eax
    127d:	89 44 24 28          	mov    %eax,0x28(%esp)
    1281:	89 c1                	mov    %eax,%ecx
    1283:	89 f8                	mov    %edi,%eax
    1285:	f7 f1                	div    %ecx
		if (mis != 0) {
    1287:	83 c4 10             	add    $0x10,%esp
	void* ptr;

	/* if no free block available */
	if (!alloc->free_block) {
		tommy_uintptr_t off, mis;
		tommy_size_t size = TOMMY_ALLOCATOR_BLOCK_SIZE;
    128a:	c7 44 24 14 c0 0f 00 	movl   $0xfc0,0x14(%esp)
    1291:	00 
		data += sizeof(tommy_allocator_entry);

		/* align if not aligned */
		off = (tommy_uintptr_t)data;
		mis = off % alloc->align_size;
		if (mis != 0) {
    1292:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
    1296:	85 d2                	test   %edx,%edx
    1298:	74 17                	je     12b1 <tommy_trie_insert+0x1c1>
			data += alloc->align_size - mis;
    129a:	8b 44 24 18          	mov    0x18(%esp),%eax
    129e:	29 d0                	sub    %edx,%eax
    12a0:	01 c7                	add    %eax,%edi
    12a2:	b8 c0 0f 00 00       	mov    $0xfc0,%eax
    12a7:	2b 44 24 18          	sub    0x18(%esp),%eax
			size -= alloc->align_size - mis;
    12ab:	01 d0                	add    %edx,%eax
    12ad:	89 44 24 14          	mov    %eax,0x14(%esp)
		}

		/* insert in free list */
		while (size >= alloc->block_size) {
    12b1:	8b 56 08             	mov    0x8(%esi),%edx
    12b4:	39 54 24 14          	cmp    %edx,0x14(%esp)
    12b8:	72 7d                	jb     1337 <tommy_trie_insert+0x247>
    12ba:	89 74 24 18          	mov    %esi,0x18(%esp)
    12be:	8b 44 24 14          	mov    0x14(%esp),%eax
    12c2:	eb 08                	jmp    12cc <tommy_trie_insert+0x1dc>
    12c4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    12c8:	89 fb                	mov    %edi,%ebx
			tommy_allocator_entry* free_block = (tommy_allocator_entry*)data;
			free_block->next = alloc->free_block;
			alloc->free_block = free_block;

			data += alloc->block_size;
    12ca:	89 f7                	mov    %esi,%edi
			size -= alloc->block_size;
    12cc:	29 d0                	sub    %edx,%eax
		}

		/* insert in free list */
		while (size >= alloc->block_size) {
			tommy_allocator_entry* free_block = (tommy_allocator_entry*)data;
			free_block->next = alloc->free_block;
    12ce:	89 1f                	mov    %ebx,(%edi)
			alloc->free_block = free_block;

			data += alloc->block_size;
    12d0:	8d 34 17             	lea    (%edi,%edx,1),%esi
			data += alloc->align_size - mis;
			size -= alloc->align_size - mis;
		}

		/* insert in free list */
		while (size >= alloc->block_size) {
    12d3:	39 d0                	cmp    %edx,%eax
    12d5:	73 f1                	jae    12c8 <tommy_trie_insert+0x1d8>
    12d7:	89 d8                	mov    %ebx,%eax
    12d9:	8b 74 24 18          	mov    0x18(%esp),%esi
    12dd:	89 fb                	mov    %edi,%ebx
    12df:	e9 be fe ff ff       	jmp    11a2 <tommy_trie_insert+0xb2>
    12e4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    12e8:	8b 6c 24 10          	mov    0x10(%esp),%ebp
	j = (key >> shift) & TOMMY_TRIE_TREE_MASK;

	/* if they don't collide */
	if (i != j) {
		/* insert the already existing element */
		tree->map[i] = node;
    12ec:	89 2c 83             	mov    %ebp,(%ebx,%eax,4)
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    12ef:	8b 44 24 44          	mov    0x44(%esp),%eax
    12f3:	89 40 04             	mov    %eax,0x4(%eax)

	/* one element "0 terminated" next list */
	node->next = 0;
    12f6:	c7 00 00 00 00 00    	movl   $0x0,(%eax)

	*list = node;
    12fc:	89 04 93             	mov    %eax,(%ebx,%edx,4)

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];

	trie_bucket_insert(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, node, key);

	++trie->count;
    12ff:	8b 44 24 40          	mov    0x40(%esp),%eax
    1303:	83 40 40 01          	addl   $0x1,0x40(%eax)
}
    1307:	83 c4 2c             	add    $0x2c,%esp
    130a:	5b                   	pop    %ebx
    130b:	5e                   	pop    %esi
    130c:	5f                   	pop    %edi
    130d:	5d                   	pop    %ebp
    130e:	c3                   	ret    
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_tail_not_empty(tommy_node* head, tommy_node* node)
{
	/* insert in the "circular" prev list */
	node->prev = head->prev;
    130f:	8b 45 04             	mov    0x4(%ebp),%eax
    1312:	8b 7c 24 44          	mov    0x44(%esp),%edi
    1316:	89 47 04             	mov    %eax,0x4(%edi)
	head->prev = node;
    1319:	89 7d 04             	mov    %edi,0x4(%ebp)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
    131c:	8b 47 04             	mov    0x4(%edi),%eax
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
    131f:	c7 07 00 00 00 00    	movl   $0x0,(%edi)
	node->prev->next = node;
    1325:	89 38                	mov    %edi,(%eax)

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];

	trie_bucket_insert(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, node, key);

	++trie->count;
    1327:	8b 44 24 40          	mov    0x40(%esp),%eax
    132b:	83 40 40 01          	addl   $0x1,0x40(%eax)
}
    132f:	83 c4 2c             	add    $0x2c,%esp
    1332:	5b                   	pop    %ebx
    1333:	5e                   	pop    %esi
    1334:	5f                   	pop    %edi
    1335:	5d                   	pop    %ebp
    1336:	c3                   	ret    
    1337:	a1 00 00 00 00       	mov    0x0,%eax
    133c:	31 db                	xor    %ebx,%ebx
    133e:	e9 5f fe ff ff       	jmp    11a2 <tommy_trie_insert+0xb2>
    1343:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1349:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001350 <tommy_trie_remove>:

	return remove;
}

void* tommy_trie_remove(tommy_trie* trie, tommy_key_t key)
{
    1350:	55                   	push   %ebp
    1351:	57                   	push   %edi
    1352:	56                   	push   %esi
    1353:	53                   	push   %ebx
    1354:	83 ec 3c             	sub    $0x3c,%esp
    1357:	8b 5c 24 54          	mov    0x54(%esp),%ebx
    135b:	8b 54 24 50          	mov    0x50(%esp),%edx
	tommy_trie_node* ret;
	tommy_trie_node** let_ptr;

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    135f:	89 de                	mov    %ebx,%esi
    1361:	c1 ee 1c             	shr    $0x1c,%esi
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1364:	8b 04 b2             	mov    (%edx,%esi,4),%eax
void* tommy_trie_remove(tommy_trie* trie, tommy_key_t key)
{
	tommy_trie_node* ret;
	tommy_trie_node** let_ptr;

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    1367:	8d 0c b2             	lea    (%edx,%esi,4),%ecx

	level = 0;
recurse:
	ptr = *let_ptr;

	if (!ptr)
    136a:	85 c0                	test   %eax,%eax
    136c:	0f 84 3e 01 00 00    	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1372:	a8 01                	test   $0x1,%al
    1374:	0f 84 d6 01 00 00    	je     1550 <tommy_trie_remove+0x200>
		tree = trie_get_tree(ptr);
    137a:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    137d:	83 e6 0f             	and    $0xf,%esi

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1380:	89 4c 24 10          	mov    %ecx,0x10(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1384:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1387:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    138a:	85 c0                	test   %eax,%eax
    138c:	0f 84 1e 01 00 00    	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1392:	a8 01                	test   $0x1,%al
    1394:	0f 84 06 02 00 00    	je     15a0 <tommy_trie_remove+0x250>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    139a:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    139c:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    139f:	89 4c 24 14          	mov    %ecx,0x14(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    13a3:	c1 ee 18             	shr    $0x18,%esi
    13a6:	83 e6 0f             	and    $0xf,%esi
    13a9:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    13ac:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    13af:	85 c0                	test   %eax,%eax
    13b1:	0f 84 f9 00 00 00    	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    13b7:	a8 01                	test   $0x1,%al
    13b9:	0f 84 d1 01 00 00    	je     1590 <tommy_trie_remove+0x240>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    13bf:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    13c1:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    13c4:	89 4c 24 18          	mov    %ecx,0x18(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    13c8:	c1 ee 14             	shr    $0x14,%esi
    13cb:	83 e6 0f             	and    $0xf,%esi
    13ce:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    13d1:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    13d4:	85 c0                	test   %eax,%eax
    13d6:	0f 84 d4 00 00 00    	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    13dc:	a8 01                	test   $0x1,%al
    13de:	0f 84 cc 01 00 00    	je     15b0 <tommy_trie_remove+0x260>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    13e4:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    13e6:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    13e9:	89 4c 24 1c          	mov    %ecx,0x1c(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    13ed:	c1 ee 10             	shr    $0x10,%esi
    13f0:	83 e6 0f             	and    $0xf,%esi
    13f3:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    13f6:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    13f9:	85 c0                	test   %eax,%eax
    13fb:	0f 84 af 00 00 00    	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1401:	a8 01                	test   $0x1,%al
    1403:	0f 84 57 01 00 00    	je     1560 <tommy_trie_remove+0x210>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1409:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    140b:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    140e:	89 4c 24 20          	mov    %ecx,0x20(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1412:	c1 ee 0c             	shr    $0xc,%esi
    1415:	83 e6 0f             	and    $0xf,%esi
    1418:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    141b:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    141e:	85 c0                	test   %eax,%eax
    1420:	0f 84 8a 00 00 00    	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1426:	a8 01                	test   $0x1,%al
    1428:	0f 84 52 01 00 00    	je     1580 <tommy_trie_remove+0x230>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    142e:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    1430:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    1433:	89 4c 24 24          	mov    %ecx,0x24(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1437:	c1 ee 08             	shr    $0x8,%esi
    143a:	83 e6 0f             	and    $0xf,%esi
    143d:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1440:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    1443:	85 c0                	test   %eax,%eax
    1445:	74 69                	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1447:	a8 01                	test   $0x1,%al
    1449:	0f 84 21 01 00 00    	je     1570 <tommy_trie_remove+0x220>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    144f:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    1451:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    1454:	89 4c 24 28          	mov    %ecx,0x28(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1458:	c1 ee 04             	shr    $0x4,%esi
    145b:	83 e6 0f             	and    $0xf,%esi
    145e:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1461:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    1464:	85 c0                	test   %eax,%eax
    1466:	74 48                	je     14b0 <tommy_trie_remove+0x160>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1468:	a8 01                	test   $0x1,%al
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    146a:	be 07 00 00 00       	mov    $0x7,%esi
	ptr = *let_ptr;

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    146f:	74 1b                	je     148c <tommy_trie_remove+0x13c>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1471:	89 de                	mov    %ebx,%esi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    1473:	83 e8 01             	sub    $0x1,%eax

		/* save the path */
		let_back[level++] = let_ptr;
    1476:	89 4c 24 2c          	mov    %ecx,0x2c(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    147a:	83 e6 0f             	and    $0xf,%esi
    147d:	8d 0c b0             	lea    (%eax,%esi,4),%ecx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1480:	8b 04 b0             	mov    (%eax,%esi,4),%eax

	if (!ptr)
    1483:	85 c0                	test   %eax,%eax
    1485:	74 29                	je     14b0 <tommy_trie_remove+0x160>

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1487:	be 08 00 00 00       	mov    $0x8,%esi
	if (!remove) {
		/* remove the first */
		remove = node;

		/* check if it's really the element to remove */
		if (remove->key != key)
    148c:	3b 58 0c             	cmp    0xc(%eax),%ebx
    148f:	75 1f                	jne    14b0 <tommy_trie_remove+0x160>
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    1491:	8b 18                	mov    (%eax),%ebx
    1493:	85 db                	test   %ebx,%ebx
    1495:	74 29                	je     14c0 <tommy_trie_remove+0x170>
		node->next->prev = node->prev;
    1497:	8b 70 04             	mov    0x4(%eax),%esi
    149a:	89 73 04             	mov    %esi,0x4(%ebx)
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
		*list = node->next; /* the new head, in case 0 */
    149d:	89 19                	mov    %ebx,(%ecx)
	ret = trie_bucket_remove_existing(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, 0, key);

	if (!ret)
		return 0;

	--trie->count;
    149f:	83 6a 40 01          	subl   $0x1,0x40(%edx)

	return ret->data;
    14a3:	8b 40 08             	mov    0x8(%eax),%eax
}
    14a6:	83 c4 3c             	add    $0x3c,%esp
    14a9:	5b                   	pop    %ebx
    14aa:	5e                   	pop    %esi
    14ab:	5f                   	pop    %edi
    14ac:	5d                   	pop    %ebp
    14ad:	c3                   	ret    
    14ae:	66 90                	xchg   %ax,%ax
    14b0:	83 c4 3c             	add    $0x3c,%esp
	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];

	ret = trie_bucket_remove_existing(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, 0, key);

	if (!ret)
		return 0;
    14b3:	31 c0                	xor    %eax,%eax

	--trie->count;

	return ret->data;
}
    14b5:	5b                   	pop    %ebx
    14b6:	5e                   	pop    %esi
    14b7:	5f                   	pop    %edi
    14b8:	5d                   	pop    %ebp
    14b9:	c3                   	ret    
    14ba:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
	}

	tommy_list_remove_existing(let_ptr, remove);

	/* if the list is not empty, try to reduce */
	if (*let_ptr || !level)
    14c0:	85 f6                	test   %esi,%esi
    14c2:	c7 01 00 00 00 00    	movl   $0x0,(%ecx)
    14c8:	74 d5                	je     149f <tommy_trie_remove+0x14f>
    14ca:	8d 7c b4 10          	lea    0x10(%esp,%esi,4),%edi
    14ce:	89 7c 24 04          	mov    %edi,0x4(%esp)
		return remove;

reduce:
	/* go one level up */
	let_ptr = let_back[--level];
    14d2:	8b 7f fc             	mov    -0x4(%edi),%edi

	tree = trie_get_tree(*let_ptr);

	/* check if there is only one child node */
	count = 0;
	last = 0;
    14d5:	31 ed                	xor    %ebp,%ebp
	let_ptr = let_back[--level];

	tree = trie_get_tree(*let_ptr);

	/* check if there is only one child node */
	count = 0;
    14d7:	31 f6                	xor    %esi,%esi
	last = 0;
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
    14d9:	31 c9                	xor    %ecx,%ecx
	if (*let_ptr || !level)
		return remove;

reduce:
	/* go one level up */
	let_ptr = let_back[--level];
    14db:	89 7c 24 08          	mov    %edi,0x8(%esp)

	tree = trie_get_tree(*let_ptr);
    14df:	8b 3f                	mov    (%edi),%edi
    14e1:	89 7c 24 0c          	mov    %edi,0xc(%esp)
    14e5:	83 ef 01             	sub    $0x1,%edi
    14e8:	90                   	nop
    14e9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

	/* check if there is only one child node */
	count = 0;
	last = 0;
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		if (tree->map[i]) {
    14f0:	8b 1c 8f             	mov    (%edi,%ecx,4),%ebx
    14f3:	85 db                	test   %ebx,%ebx
    14f5:	74 11                	je     1508 <tommy_trie_remove+0x1b8>
			/* if we have a sub tree, we cannot reduce */
			if (trie_get_type(tree->map[i]) != TOMMY_TRIE_TYPE_NODE)
    14f7:	83 e3 01             	and    $0x1,%ebx
    14fa:	75 a3                	jne    149f <tommy_trie_remove+0x14f>
				return remove;
			/* if more than one node, we cannot reduce */
			if (++count > 1)
    14fc:	83 fe 01             	cmp    $0x1,%esi
    14ff:	74 9e                	je     149f <tommy_trie_remove+0x14f>
    1501:	89 cd                	mov    %ecx,%ebp
    1503:	be 01 00 00 00       	mov    $0x1,%esi
	tree = trie_get_tree(*let_ptr);

	/* check if there is only one child node */
	count = 0;
	last = 0;
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
    1508:	83 c1 01             	add    $0x1,%ecx
    150b:	83 f9 10             	cmp    $0x10,%ecx
    150e:	75 e0                	jne    14f0 <tommy_trie_remove+0x1a0>
			last = i;
		}
	}

	/* here count is never 0, as we cannot have a tree with only one sub node */
	assert(count == 1);
    1510:	83 fe 01             	cmp    $0x1,%esi
    1513:	0f 85 a1 00 00 00    	jne    15ba <tommy_trie_remove+0x26a>

	*let_ptr = tree->map[last];
    1519:	8b 0c af             	mov    (%edi,%ebp,4),%ecx
    151c:	8b 5c 24 08          	mov    0x8(%esp),%ebx
void tommy_allocator_free(tommy_allocator* alloc, void* ptr)
{
	tommy_allocator_entry* free_block = tommy_cast(tommy_allocator_entry*, ptr);
	
	/* put it in the free list */
	free_block->next = alloc->free_block;
    1520:	8b 74 24 0c          	mov    0xc(%esp),%esi
    1524:	83 6c 24 04 04       	subl   $0x4,0x4(%esp)
    1529:	89 0b                	mov    %ecx,(%ebx)

	tommy_allocator_free(trie->alloc, tree);
    152b:	8b 4a 48             	mov    0x48(%edx),%ecx
    152e:	8b 19                	mov    (%ecx),%ebx
    1530:	89 5e ff             	mov    %ebx,-0x1(%esi)
	alloc->free_block = free_block;
    1533:	89 39                	mov    %edi,(%ecx)
	--trie->node_count;

	/* repeat until more level */
	if (level)
    1535:	8d 5c 24 10          	lea    0x10(%esp),%ebx
    1539:	8b 7c 24 04          	mov    0x4(%esp),%edi

	--alloc->count;
    153d:	83 69 10 01          	subl   $0x1,0x10(%ecx)
	assert(count == 1);

	*let_ptr = tree->map[last];

	tommy_allocator_free(trie->alloc, tree);
	--trie->node_count;
    1541:	83 6a 44 01          	subl   $0x1,0x44(%edx)

	/* repeat until more level */
	if (level)
    1545:	39 df                	cmp    %ebx,%edi
    1547:	75 89                	jne    14d2 <tommy_trie_remove+0x182>
    1549:	e9 51 ff ff ff       	jmp    149f <tommy_trie_remove+0x14f>
    154e:	66 90                	xchg   %ax,%ax
	unsigned level;
	unsigned i;
	unsigned count;
	unsigned last;

	level = 0;
    1550:	31 f6                	xor    %esi,%esi
    1552:	e9 35 ff ff ff       	jmp    148c <tommy_trie_remove+0x13c>
    1557:	89 f6                	mov    %esi,%esi
    1559:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1560:	be 04 00 00 00       	mov    $0x4,%esi
    1565:	e9 22 ff ff ff       	jmp    148c <tommy_trie_remove+0x13c>
    156a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1570:	be 06 00 00 00       	mov    $0x6,%esi
    1575:	e9 12 ff ff ff       	jmp    148c <tommy_trie_remove+0x13c>
    157a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1580:	be 05 00 00 00       	mov    $0x5,%esi
    1585:	e9 02 ff ff ff       	jmp    148c <tommy_trie_remove+0x13c>
    158a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1590:	be 02 00 00 00       	mov    $0x2,%esi
    1595:	e9 f2 fe ff ff       	jmp    148c <tommy_trie_remove+0x13c>
    159a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    15a0:	be 01 00 00 00       	mov    $0x1,%esi
    15a5:	e9 e2 fe ff ff       	jmp    148c <tommy_trie_remove+0x13c>
    15aa:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    15b0:	be 03 00 00 00       	mov    $0x3,%esi
    15b5:	e9 d2 fe ff ff       	jmp    148c <tommy_trie_remove+0x13c>
			last = i;
		}
	}

	/* here count is never 0, as we cannot have a tree with only one sub node */
	assert(count == 1);
    15ba:	68 be 00 00 00       	push   $0xbe
    15bf:	68 ee 00 00 00       	push   $0xee
    15c4:	68 1f 00 00 00       	push   $0x1f
    15c9:	68 2b 00 00 00       	push   $0x2b
    15ce:	e8 fc ff ff ff       	call   15cf <tommy_trie_remove+0x27f>
    15d3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    15d9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000015e0 <tommy_trie_remove_existing>:

	return ret->data;
}

void* tommy_trie_remove_existing(tommy_trie* trie, tommy_trie_node* node)
{
    15e0:	55                   	push   %ebp
    15e1:	57                   	push   %edi
    15e2:	56                   	push   %esi
    15e3:	53                   	push   %ebx
    15e4:	83 ec 3c             	sub    $0x3c,%esp
    15e7:	8b 44 24 54          	mov    0x54(%esp),%eax
    15eb:	8b 54 24 50          	mov    0x50(%esp),%edx
	tommy_trie_node* ret;
	tommy_key_t key = node->key;
    15ef:	8b 70 0c             	mov    0xc(%eax),%esi
	tommy_trie_node** let_ptr;

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    15f2:	89 f7                	mov    %esi,%edi
    15f4:	c1 ef 1c             	shr    $0x1c,%edi
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    15f7:	8b 0c ba             	mov    (%edx,%edi,4),%ecx
{
	tommy_trie_node* ret;
	tommy_key_t key = node->key;
	tommy_trie_node** let_ptr;

	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    15fa:	8d 1c ba             	lea    (%edx,%edi,4),%ebx

	level = 0;
recurse:
	ptr = *let_ptr;

	if (!ptr)
    15fd:	85 c9                	test   %ecx,%ecx
    15ff:	0f 84 63 01 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1605:	f6 c1 01             	test   $0x1,%cl
    1608:	0f 84 8a 01 00 00    	je     1798 <tommy_trie_remove_existing+0x1b8>
		tree = trie_get_tree(ptr);
    160e:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1611:	83 e7 0f             	and    $0xf,%edi

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1614:	89 5c 24 10          	mov    %ebx,0x10(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1618:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    161b:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    161e:	85 c9                	test   %ecx,%ecx
    1620:	0f 84 42 01 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1626:	f6 c1 01             	test   $0x1,%cl
    1629:	0f 84 21 02 00 00    	je     1850 <tommy_trie_remove_existing+0x270>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    162f:	89 f7                	mov    %esi,%edi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    1631:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;
    1634:	89 5c 24 14          	mov    %ebx,0x14(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1638:	c1 ef 18             	shr    $0x18,%edi
    163b:	83 e7 0f             	and    $0xf,%edi
    163e:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1641:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    1644:	85 c9                	test   %ecx,%ecx
    1646:	0f 84 1c 01 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    164c:	f6 c1 01             	test   $0x1,%cl
    164f:	0f 84 3b 02 00 00    	je     1890 <tommy_trie_remove_existing+0x2b0>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1655:	89 f7                	mov    %esi,%edi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    1657:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;
    165a:	89 5c 24 18          	mov    %ebx,0x18(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    165e:	c1 ef 14             	shr    $0x14,%edi
    1661:	83 e7 0f             	and    $0xf,%edi
    1664:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    1667:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    166a:	85 c9                	test   %ecx,%ecx
    166c:	0f 84 f6 00 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1672:	f6 c1 01             	test   $0x1,%cl
    1675:	0f 84 05 02 00 00    	je     1880 <tommy_trie_remove_existing+0x2a0>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    167b:	89 f7                	mov    %esi,%edi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    167d:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;
    1680:	89 5c 24 1c          	mov    %ebx,0x1c(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1684:	c1 ef 10             	shr    $0x10,%edi
    1687:	83 e7 0f             	and    $0xf,%edi
    168a:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    168d:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    1690:	85 c9                	test   %ecx,%ecx
    1692:	0f 84 d0 00 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1698:	f6 c1 01             	test   $0x1,%cl
    169b:	0f 84 bf 01 00 00    	je     1860 <tommy_trie_remove_existing+0x280>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    16a1:	89 f7                	mov    %esi,%edi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    16a3:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;
    16a6:	89 5c 24 20          	mov    %ebx,0x20(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    16aa:	c1 ef 0c             	shr    $0xc,%edi
    16ad:	83 e7 0f             	and    $0xf,%edi
    16b0:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    16b3:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    16b6:	85 c9                	test   %ecx,%ecx
    16b8:	0f 84 aa 00 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    16be:	f6 c1 01             	test   $0x1,%cl
    16c1:	0f 84 a9 01 00 00    	je     1870 <tommy_trie_remove_existing+0x290>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    16c7:	89 f7                	mov    %esi,%edi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    16c9:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;
    16cc:	89 5c 24 24          	mov    %ebx,0x24(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    16d0:	c1 ef 08             	shr    $0x8,%edi
    16d3:	83 e7 0f             	and    $0xf,%edi
    16d6:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    16d9:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    16dc:	85 c9                	test   %ecx,%ecx
    16de:	0f 84 84 00 00 00    	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    16e4:	f6 c1 01             	test   $0x1,%cl
    16e7:	0f 84 53 01 00 00    	je     1840 <tommy_trie_remove_existing+0x260>

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    16ed:	89 f7                	mov    %esi,%edi

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);
    16ef:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;
    16f2:	89 5c 24 28          	mov    %ebx,0x28(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    16f6:	c1 ef 04             	shr    $0x4,%edi
    16f9:	83 e7 0f             	and    $0xf,%edi
    16fc:	8d 1c b9             	lea    (%ecx,%edi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    16ff:	8b 0c b9             	mov    (%ecx,%edi,4),%ecx

	if (!ptr)
    1702:	85 c9                	test   %ecx,%ecx
    1704:	74 62                	je     1768 <tommy_trie_remove_existing+0x188>
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1706:	f6 c1 01             	test   $0x1,%cl
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1709:	bf 07 00 00 00       	mov    $0x7,%edi
	ptr = *let_ptr;

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    170e:	74 19                	je     1729 <tommy_trie_remove_existing+0x149>
		tree = trie_get_tree(ptr);
    1710:	83 e9 01             	sub    $0x1,%ecx

		/* save the path */
		let_back[level++] = let_ptr;

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    1713:	83 e6 0f             	and    $0xf,%esi

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1716:	89 5c 24 2c          	mov    %ebx,0x2c(%esp)

		/* go down one level */
		let_ptr = &tree->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    171a:	8d 1c b1             	lea    (%ecx,%esi,4),%ebx
	unsigned count;
	unsigned last;

	level = 0;
recurse:
	ptr = *let_ptr;
    171d:	8b 0c b1             	mov    (%ecx,%esi,4),%ecx

	if (!ptr)
    1720:	85 c9                	test   %ecx,%ecx
    1722:	74 44                	je     1768 <tommy_trie_remove_existing+0x188>

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1724:	bf 08 00 00 00       	mov    $0x8,%edi
    1729:	c6 44 24 04 00       	movb   $0x0,0x4(%esp)
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    172e:	8b 30                	mov    (%eax),%esi
		node->next->prev = node->prev;
    1730:	8b 68 04             	mov    0x4(%eax),%ebp
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    1733:	85 f6                	test   %esi,%esi
    1735:	74 51                	je     1788 <tommy_trie_remove_existing+0x1a8>
		node->next->prev = node->prev;
    1737:	89 6e 04             	mov    %ebp,0x4(%esi)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    173a:	39 c8                	cmp    %ecx,%eax
    173c:	74 52                	je     1790 <tommy_trie_remove_existing+0x1b0>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    173e:	8b 48 04             	mov    0x4(%eax),%ecx
    1741:	89 31                	mov    %esi,(%ecx)
    1743:	8b 33                	mov    (%ebx),%esi
	}

	tommy_list_remove_existing(let_ptr, remove);

	/* if the list is not empty, try to reduce */
	if (*let_ptr || !level)
    1745:	85 f6                	test   %esi,%esi
    1747:	75 09                	jne    1752 <tommy_trie_remove_existing+0x172>
    1749:	0f b6 5c 24 04       	movzbl 0x4(%esp),%ebx
    174e:	84 db                	test   %bl,%bl
    1750:	74 56                	je     17a8 <tommy_trie_remove_existing+0x1c8>
	ret = trie_bucket_remove_existing(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, node, key);

	/* the element removed must match the one passed */
	assert(ret == node);

	--trie->count;
    1752:	83 6a 40 01          	subl   $0x1,0x40(%edx)

	return ret->data;
    1756:	8b 40 08             	mov    0x8(%eax),%eax
}
    1759:	83 c4 3c             	add    $0x3c,%esp
    175c:	5b                   	pop    %ebx
    175d:	5e                   	pop    %esi
    175e:	5f                   	pop    %edi
    175f:	5d                   	pop    %ebp
    1760:	c3                   	ret    
    1761:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
	let_ptr = &trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];

	ret = trie_bucket_remove_existing(trie, TOMMY_TRIE_BUCKET_SHIFT, let_ptr, node, key);

	/* the element removed must match the one passed */
	assert(ret == node);
    1768:	68 a3 00 00 00       	push   $0xa3
    176d:	68 18 01 00 00       	push   $0x118
    1772:	68 1f 00 00 00       	push   $0x1f
    1777:	68 36 00 00 00       	push   $0x36
    177c:	e8 fc ff ff ff       	call   177d <tommy_trie_remove_existing+0x19d>
    1781:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    1788:	89 69 04             	mov    %ebp,0x4(%ecx)
    178b:	eb ad                	jmp    173a <tommy_trie_remove_existing+0x15a>
    178d:	8d 76 00             	lea    0x0(%esi),%esi
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
		*list = node->next; /* the new head, in case 0 */
    1790:	89 33                	mov    %esi,(%ebx)
    1792:	eb b1                	jmp    1745 <tommy_trie_remove_existing+0x165>
    1794:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
	ptr = *let_ptr;

	if (!ptr)
		return 0;

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
    1798:	c6 44 24 04 01       	movb   $0x1,0x4(%esp)
	unsigned level;
	unsigned i;
	unsigned count;
	unsigned last;

	level = 0;
    179d:	31 ff                	xor    %edi,%edi
    179f:	eb 8d                	jmp    172e <tommy_trie_remove_existing+0x14e>
    17a1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    17a8:	8d 74 bc 10          	lea    0x10(%esp,%edi,4),%esi
    17ac:	89 74 24 04          	mov    %esi,0x4(%esp)
	if (*let_ptr || !level)
		return remove;

reduce:
	/* go one level up */
	let_ptr = let_back[--level];
    17b0:	8b 76 fc             	mov    -0x4(%esi),%esi

	tree = trie_get_tree(*let_ptr);

	/* check if there is only one child node */
	count = 0;
	last = 0;
    17b3:	31 ed                	xor    %ebp,%ebp
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
    17b5:	31 c9                	xor    %ecx,%ecx
	if (*let_ptr || !level)
		return remove;

reduce:
	/* go one level up */
	let_ptr = let_back[--level];
    17b7:	89 74 24 08          	mov    %esi,0x8(%esp)

	tree = trie_get_tree(*let_ptr);
    17bb:	8b 36                	mov    (%esi),%esi
    17bd:	89 74 24 0c          	mov    %esi,0xc(%esp)
    17c1:	8d 7e ff             	lea    -0x1(%esi),%edi

	/* check if there is only one child node */
	count = 0;
    17c4:	31 f6                	xor    %esi,%esi
    17c6:	8d 76 00             	lea    0x0(%esi),%esi
    17c9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
	last = 0;
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
		if (tree->map[i]) {
    17d0:	8b 1c 8f             	mov    (%edi,%ecx,4),%ebx
    17d3:	85 db                	test   %ebx,%ebx
    17d5:	74 19                	je     17f0 <tommy_trie_remove_existing+0x210>
			/* if we have a sub tree, we cannot reduce */
			if (trie_get_type(tree->map[i]) != TOMMY_TRIE_TYPE_NODE)
    17d7:	83 e3 01             	and    $0x1,%ebx
    17da:	0f 85 72 ff ff ff    	jne    1752 <tommy_trie_remove_existing+0x172>
				return remove;
			/* if more than one node, we cannot reduce */
			if (++count > 1)
    17e0:	83 fe 01             	cmp    $0x1,%esi
    17e3:	0f 84 69 ff ff ff    	je     1752 <tommy_trie_remove_existing+0x172>
    17e9:	89 cd                	mov    %ecx,%ebp
    17eb:	be 01 00 00 00       	mov    $0x1,%esi
	tree = trie_get_tree(*let_ptr);

	/* check if there is only one child node */
	count = 0;
	last = 0;
	for(i=0;i<TOMMY_TRIE_TREE_MAX;++i) {
    17f0:	83 c1 01             	add    $0x1,%ecx
    17f3:	83 f9 10             	cmp    $0x10,%ecx
    17f6:	75 d8                	jne    17d0 <tommy_trie_remove_existing+0x1f0>
			last = i;
		}
	}

	/* here count is never 0, as we cannot have a tree with only one sub node */
	assert(count == 1);
    17f8:	83 fe 01             	cmp    $0x1,%esi
    17fb:	0f 85 99 00 00 00    	jne    189a <tommy_trie_remove_existing+0x2ba>

	*let_ptr = tree->map[last];
    1801:	8b 0c af             	mov    (%edi,%ebp,4),%ecx
    1804:	8b 74 24 08          	mov    0x8(%esp),%esi
    1808:	83 6c 24 04 04       	subl   $0x4,0x4(%esp)
    180d:	89 0e                	mov    %ecx,(%esi)

	tommy_allocator_free(trie->alloc, tree);
    180f:	8b 4a 48             	mov    0x48(%edx),%ecx
void tommy_allocator_free(tommy_allocator* alloc, void* ptr)
{
	tommy_allocator_entry* free_block = tommy_cast(tommy_allocator_entry*, ptr);
	
	/* put it in the free list */
	free_block->next = alloc->free_block;
    1812:	8b 74 24 0c          	mov    0xc(%esp),%esi
    1816:	8b 19                	mov    (%ecx),%ebx
    1818:	89 5e ff             	mov    %ebx,-0x1(%esi)
    181b:	8b 74 24 04          	mov    0x4(%esp),%esi
	--trie->node_count;

	/* repeat until more level */
	if (level)
    181f:	8d 5c 24 10          	lea    0x10(%esp),%ebx
	alloc->free_block = free_block;

	--alloc->count;
    1823:	83 69 10 01          	subl   $0x1,0x10(%ecx)
{
	tommy_allocator_entry* free_block = tommy_cast(tommy_allocator_entry*, ptr);
	
	/* put it in the free list */
	free_block->next = alloc->free_block;
	alloc->free_block = free_block;
    1827:	89 39                	mov    %edi,(%ecx)
	assert(count == 1);

	*let_ptr = tree->map[last];

	tommy_allocator_free(trie->alloc, tree);
	--trie->node_count;
    1829:	83 6a 44 01          	subl   $0x1,0x44(%edx)

	/* repeat until more level */
	if (level)
    182d:	39 de                	cmp    %ebx,%esi
    182f:	0f 85 7b ff ff ff    	jne    17b0 <tommy_trie_remove_existing+0x1d0>
    1835:	e9 18 ff ff ff       	jmp    1752 <tommy_trie_remove_existing+0x172>
    183a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi

	if (trie_get_type(ptr) == TOMMY_TRIE_TYPE_TREE) {
		tree = trie_get_tree(ptr);

		/* save the path */
		let_back[level++] = let_ptr;
    1840:	bf 06 00 00 00       	mov    $0x6,%edi
    1845:	e9 df fe ff ff       	jmp    1729 <tommy_trie_remove_existing+0x149>
    184a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1850:	bf 01 00 00 00       	mov    $0x1,%edi
    1855:	e9 cf fe ff ff       	jmp    1729 <tommy_trie_remove_existing+0x149>
    185a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1860:	bf 04 00 00 00       	mov    $0x4,%edi
    1865:	e9 bf fe ff ff       	jmp    1729 <tommy_trie_remove_existing+0x149>
    186a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1870:	bf 05 00 00 00       	mov    $0x5,%edi
    1875:	e9 af fe ff ff       	jmp    1729 <tommy_trie_remove_existing+0x149>
    187a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1880:	bf 03 00 00 00       	mov    $0x3,%edi
    1885:	e9 9f fe ff ff       	jmp    1729 <tommy_trie_remove_existing+0x149>
    188a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1890:	bf 02 00 00 00       	mov    $0x2,%edi
    1895:	e9 8f fe ff ff       	jmp    1729 <tommy_trie_remove_existing+0x149>
			last = i;
		}
	}

	/* here count is never 0, as we cannot have a tree with only one sub node */
	assert(count == 1);
    189a:	68 be 00 00 00       	push   $0xbe
    189f:	68 ee 00 00 00       	push   $0xee
    18a4:	68 1f 00 00 00       	push   $0x1f
    18a9:	68 2b 00 00 00       	push   $0x2b
    18ae:	e8 fc ff ff ff       	call   18af <tommy_trie_remove_existing+0x2cf>
    18b3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    18b9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000018c0 <tommy_trie_bucket>:

	return ret->data;
}

tommy_trie_node* tommy_trie_bucket(tommy_trie* trie, tommy_key_t key)
{
    18c0:	53                   	push   %ebx
    18c1:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
	tommy_trie_node* node;
	void* ptr;
	unsigned type;
	unsigned shift;

	ptr = trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];
    18c5:	8b 44 24 08          	mov    0x8(%esp),%eax
    18c9:	89 da                	mov    %ebx,%edx
    18cb:	c1 ea 1c             	shr    $0x1c,%edx
    18ce:	8b 04 90             	mov    (%eax,%edx,4),%eax

	shift = TOMMY_TRIE_BUCKET_SHIFT;

recurse:
	if (!ptr)
    18d1:	85 c0                	test   %eax,%eax
    18d3:	74 2c                	je     1901 <tommy_trie_bucket+0x41>
    18d5:	b9 1c 00 00 00       	mov    $0x1c,%ecx
    18da:	eb 16                	jmp    18f2 <tommy_trie_bucket+0x32>
    18dc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
		if (node->key != key)
			return 0;
		return node;
	default:
	case TOMMY_TRIE_TYPE_TREE :
		ptr = trie_get_tree(ptr)->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    18e0:	89 da                	mov    %ebx,%edx
    18e2:	d3 ea                	shr    %cl,%edx
		shift -= TOMMY_TRIE_TREE_BIT;
    18e4:	83 e9 04             	sub    $0x4,%ecx
		if (node->key != key)
			return 0;
		return node;
	default:
	case TOMMY_TRIE_TYPE_TREE :
		ptr = trie_get_tree(ptr)->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
    18e7:	83 e2 0f             	and    $0xf,%edx
    18ea:	8b 44 90 ff          	mov    -0x1(%eax,%edx,4),%eax
	ptr = trie->bucket[key >> TOMMY_TRIE_BUCKET_SHIFT];

	shift = TOMMY_TRIE_BUCKET_SHIFT;

recurse:
	if (!ptr)
    18ee:	85 c0                	test   %eax,%eax
    18f0:	74 0d                	je     18ff <tommy_trie_bucket+0x3f>
		return 0;

	type = trie_get_type(ptr);

	switch (type) {
    18f2:	89 c2                	mov    %eax,%edx
    18f4:	83 e2 01             	and    $0x1,%edx
    18f7:	75 e7                	jne    18e0 <tommy_trie_bucket+0x20>
	case TOMMY_TRIE_TYPE_NODE :
		node = tommy_cast(tommy_trie_node*, ptr);
		if (node->key != key)
    18f9:	39 58 0c             	cmp    %ebx,0xc(%eax)
    18fc:	0f 45 c2             	cmovne %edx,%eax
	case TOMMY_TRIE_TYPE_TREE :
		ptr = trie_get_tree(ptr)->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
		shift -= TOMMY_TRIE_TREE_BIT;
		goto recurse;
	}
}
    18ff:	5b                   	pop    %ebx
    1900:	c3                   	ret    

	shift = TOMMY_TRIE_BUCKET_SHIFT;

recurse:
	if (!ptr)
		return 0;
    1901:	31 c0                	xor    %eax,%eax
	case TOMMY_TRIE_TYPE_TREE :
		ptr = trie_get_tree(ptr)->map[(key >> shift) & TOMMY_TRIE_TREE_MASK];
		shift -= TOMMY_TRIE_TREE_BIT;
		goto recurse;
	}
}
    1903:	5b                   	pop    %ebx
    1904:	c3                   	ret    
    1905:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1909:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001910 <tommy_trie_memory_usage>:

tommy_size_t tommy_trie_memory_usage(tommy_trie* trie)
{
    1910:	8b 44 24 04          	mov    0x4(%esp),%eax
	return tommy_trie_count(trie) * (tommy_size_t)sizeof(tommy_trie_node)
		+ trie->node_count * (tommy_size_t)TOMMY_TRIE_BLOCK_SIZE;
    1914:	8b 50 44             	mov    0x44(%eax),%edx
    1917:	8b 40 40             	mov    0x40(%eax),%eax
    191a:	8d 04 90             	lea    (%eax,%edx,4),%eax
	}
}

tommy_size_t tommy_trie_memory_usage(tommy_trie* trie)
{
	return tommy_trie_count(trie) * (tommy_size_t)sizeof(tommy_trie_node)
    191d:	c1 e0 04             	shl    $0x4,%eax
		+ trie->node_count * (tommy_size_t)TOMMY_TRIE_BLOCK_SIZE;
}
    1920:	c3                   	ret    
    1921:	eb 0d                	jmp    1930 <tommy_trie_inplace_init>
    1923:	90                   	nop
    1924:	90                   	nop
    1925:	90                   	nop
    1926:	90                   	nop
    1927:	90                   	nop
    1928:	90                   	nop
    1929:	90                   	nop
    192a:	90                   	nop
    192b:	90                   	nop
    192c:	90                   	nop
    192d:	90                   	nop
    192e:	90                   	nop
    192f:	90                   	nop

00001930 <tommy_trie_inplace_init>:
		node->prev->next = node->next;
	}
}

void tommy_trie_inplace_init(tommy_trie_inplace* trie_inplace)
{
    1930:	57                   	push   %edi
    1931:	8b 54 24 08          	mov    0x8(%esp),%edx
    1935:	31 c0                	xor    %eax,%eax
    1937:	8d 7a 04             	lea    0x4(%edx),%edi
    193a:	89 d1                	mov    %edx,%ecx
    193c:	c7 02 00 00 00 00    	movl   $0x0,(%edx)
    1942:	c7 82 fc 00 00 00 00 	movl   $0x0,0xfc(%edx)
    1949:	00 00 00 
    194c:	83 e7 fc             	and    $0xfffffffc,%edi
    194f:	29 f9                	sub    %edi,%ecx
    1951:	81 c1 00 01 00 00    	add    $0x100,%ecx
    1957:	c1 e9 02             	shr    $0x2,%ecx
    195a:	f3 ab                	rep stos %eax,%es:(%edi)
	unsigned i;

	for(i=0;i<TOMMY_TRIE_INPLACE_BUCKET_MAX;++i)
		trie_inplace->bucket[i] = 0;
	
	trie_inplace->count = 0;
    195c:	c7 82 00 01 00 00 00 	movl   $0x0,0x100(%edx)
    1963:	00 00 00 
}
    1966:	5f                   	pop    %edi
    1967:	c3                   	ret    
    1968:	90                   	nop
    1969:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00001970 <tommy_trie_inplace_insert>:
		tommy_trie_inplace_list_insert_tail_not_empty(node, insert);
	}
}

void tommy_trie_inplace_insert(tommy_trie_inplace* trie_inplace, tommy_trie_inplace_node* node, void* data, tommy_key_t key)
{
    1970:	55                   	push   %ebp
    1971:	57                   	push   %edi
    1972:	56                   	push   %esi
    1973:	53                   	push   %ebx
    1974:	8b 74 24 18          	mov    0x18(%esp),%esi
    1978:	8b 54 24 20          	mov    0x20(%esp),%edx
	tommy_trie_inplace_node** let_ptr;
	unsigned i;

	node->data = data;
    197c:	8b 44 24 1c          	mov    0x1c(%esp),%eax
		tommy_trie_inplace_list_insert_tail_not_empty(node, insert);
	}
}

void tommy_trie_inplace_insert(tommy_trie_inplace* trie_inplace, tommy_trie_inplace_node* node, void* data, tommy_key_t key)
{
    1980:	8b 7c 24 14          	mov    0x14(%esp),%edi

	node->data = data;
	node->key = key;
	/* clear the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i)
		node->map[i] = 0;
    1984:	c7 46 10 00 00 00 00 	movl   $0x0,0x10(%esi)
    198b:	c7 46 14 00 00 00 00 	movl   $0x0,0x14(%esi)
void tommy_trie_inplace_insert(tommy_trie_inplace* trie_inplace, tommy_trie_inplace_node* node, void* data, tommy_key_t key)
{
	tommy_trie_inplace_node** let_ptr;
	unsigned i;

	node->data = data;
    1992:	89 46 08             	mov    %eax,0x8(%esi)
	node->key = key;
	/* clear the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i)
		node->map[i] = 0;

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1995:	89 d0                	mov    %edx,%eax

	node->data = data;
	node->key = key;
	/* clear the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i)
		node->map[i] = 0;
    1997:	c7 46 18 00 00 00 00 	movl   $0x0,0x18(%esi)

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    199e:	c1 e8 1a             	shr    $0x1a,%eax

	node->data = data;
	node->key = key;
	/* clear the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i)
		node->map[i] = 0;
    19a1:	c7 46 1c 00 00 00 00 	movl   $0x0,0x1c(%esi)
{
	tommy_trie_inplace_node** let_ptr;
	unsigned i;

	node->data = data;
	node->key = key;
    19a8:	89 56 0c             	mov    %edx,0xc(%esi)

static void trie_inplace_bucket_insert(unsigned shift, tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* insert, tommy_key_t key)
{
	tommy_trie_inplace_node* node;

	node = *let_ptr;
    19ab:	8b 1c 87             	mov    (%edi,%eax,4),%ebx
	while (node && node->key != key) {
    19ae:	85 db                	test   %ebx,%ebx
    19b0:	74 6e                	je     1a20 <tommy_trie_inplace_insert+0xb0>
    19b2:	3b 53 0c             	cmp    0xc(%ebx),%edx
    19b5:	74 3e                	je     19f5 <tommy_trie_inplace_insert+0x85>
    19b7:	b9 1a 00 00 00       	mov    $0x1a,%ecx
    19bc:	eb 09                	jmp    19c7 <tommy_trie_inplace_insert+0x57>
    19be:	66 90                	xchg   %ax,%ax
    19c0:	3b 55 0c             	cmp    0xc(%ebp),%edx
    19c3:	89 eb                	mov    %ebp,%ebx
    19c5:	74 39                	je     1a00 <tommy_trie_inplace_insert+0x90>
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    19c7:	89 d0                	mov    %edx,%eax
    19c9:	d3 e8                	shr    %cl,%eax
		node = *let_ptr;
		shift -= TOMMY_TRIE_INPLACE_TREE_BIT;
    19cb:	83 e9 02             	sub    $0x2,%ecx
{
	tommy_trie_inplace_node* node;

	node = *let_ptr;
	while (node && node->key != key) {
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    19ce:	83 e0 03             	and    $0x3,%eax
		node = *let_ptr;
    19d1:	83 c0 04             	add    $0x4,%eax
    19d4:	8b 2c 83             	mov    (%ebx,%eax,4),%ebp
static void trie_inplace_bucket_insert(unsigned shift, tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* insert, tommy_key_t key)
{
	tommy_trie_inplace_node* node;

	node = *let_ptr;
	while (node && node->key != key) {
    19d7:	85 ed                	test   %ebp,%ebp
    19d9:	75 e5                	jne    19c0 <tommy_trie_inplace_insert+0x50>
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    19db:	8d 04 83             	lea    (%ebx,%eax,4),%eax
 * Create a new list with a single element.
 */
tommy_inline tommy_trie_inplace_node* tommy_trie_inplace_list_insert_first(tommy_trie_inplace_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    19de:	89 76 04             	mov    %esi,0x4(%esi)

	/* one element "0 terminated" next list */
	node->next = 0;
    19e1:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
	}

	/* if null, just insert the node */
	if (!node) {
		/* setup the node as a list */
		*let_ptr = tommy_trie_inplace_list_insert_first(insert);
    19e7:	89 30                	mov    %esi,(%eax)

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];

	trie_inplace_bucket_insert(TOMMY_TRIE_INPLACE_BUCKET_SHIFT, let_ptr, node, key);

	++trie_inplace->count;
    19e9:	83 87 00 01 00 00 01 	addl   $0x1,0x100(%edi)
}
    19f0:	5b                   	pop    %ebx
    19f1:	5e                   	pop    %esi
    19f2:	5f                   	pop    %edi
    19f3:	5d                   	pop    %ebp
    19f4:	c3                   	ret    
static void trie_inplace_bucket_insert(unsigned shift, tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* insert, tommy_key_t key)
{
	tommy_trie_inplace_node* node;

	node = *let_ptr;
	while (node && node->key != key) {
    19f5:	89 dd                	mov    %ebx,%ebp
    19f7:	89 f6                	mov    %esi,%esi
    19f9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
tommy_inline void tommy_trie_inplace_list_insert_tail_not_empty(tommy_trie_inplace_node* head, tommy_trie_inplace_node* node)
{
	/* insert in the list in the last position */

	/* insert in the "circular" prev list */
	node->prev = head->prev;
    1a00:	8b 45 04             	mov    0x4(%ebp),%eax
    1a03:	89 46 04             	mov    %eax,0x4(%esi)
	head->prev = node;
    1a06:	89 75 04             	mov    %esi,0x4(%ebp)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
    1a09:	8b 46 04             	mov    0x4(%esi),%eax
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
    1a0c:	c7 06 00 00 00 00    	movl   $0x0,(%esi)
	node->prev->next = node;
    1a12:	89 30                	mov    %esi,(%eax)

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];

	trie_inplace_bucket_insert(TOMMY_TRIE_INPLACE_BUCKET_SHIFT, let_ptr, node, key);

	++trie_inplace->count;
    1a14:	83 87 00 01 00 00 01 	addl   $0x1,0x100(%edi)
}
    1a1b:	5b                   	pop    %ebx
    1a1c:	5e                   	pop    %esi
    1a1d:	5f                   	pop    %edi
    1a1e:	5d                   	pop    %ebp
    1a1f:	c3                   	ret    
	node->key = key;
	/* clear the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i)
		node->map[i] = 0;

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1a20:	8d 04 87             	lea    (%edi,%eax,4),%eax
    1a23:	eb b9                	jmp    19de <tommy_trie_inplace_insert+0x6e>
    1a25:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1a29:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001a30 <tommy_trie_inplace_remove>:

	return remove;
}

void* tommy_trie_inplace_remove(tommy_trie_inplace* trie_inplace, tommy_key_t key)
{
    1a30:	55                   	push   %ebp
    1a31:	57                   	push   %edi
    1a32:	56                   	push   %esi
    1a33:	53                   	push   %ebx
    1a34:	83 ec 04             	sub    $0x4,%esp
    1a37:	8b 7c 24 1c          	mov    0x1c(%esp),%edi
    1a3b:	8b 74 24 18          	mov    0x18(%esp),%esi
	tommy_trie_inplace_node* ret;
	tommy_trie_inplace_node** let_ptr;

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1a3f:	89 f8                	mov    %edi,%eax
    1a41:	c1 e8 1a             	shr    $0x1a,%eax
	tommy_trie_inplace_node* node;
	int i;
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
    1a44:	8b 1c 86             	mov    (%esi,%eax,4),%ebx
	while (node && node->key != key) {
    1a47:	85 db                	test   %ebx,%ebx
    1a49:	74 30                	je     1a7b <tommy_trie_inplace_remove+0x4b>
    1a4b:	3b 7b 0c             	cmp    0xc(%ebx),%edi
    1a4e:	0f 84 f5 00 00 00    	je     1b49 <tommy_trie_inplace_remove+0x119>
    1a54:	b9 1a 00 00 00       	mov    $0x1a,%ecx
    1a59:	eb 0c                	jmp    1a67 <tommy_trie_inplace_remove+0x37>
    1a5b:	90                   	nop
    1a5c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1a60:	3b 78 0c             	cmp    0xc(%eax),%edi
    1a63:	74 23                	je     1a88 <tommy_trie_inplace_remove+0x58>
    1a65:	89 c3                	mov    %eax,%ebx
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1a67:	89 fa                	mov    %edi,%edx
    1a69:	d3 ea                	shr    %cl,%edx
		node = *let_ptr;
		shift -= TOMMY_TRIE_INPLACE_TREE_BIT;
    1a6b:	83 e9 02             	sub    $0x2,%ecx
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
	while (node && node->key != key) {
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1a6e:	83 e2 03             	and    $0x3,%edx
		node = *let_ptr;
    1a71:	83 c2 04             	add    $0x4,%edx
    1a74:	8b 04 93             	mov    (%ebx,%edx,4),%eax
	int i;
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
	while (node && node->key != key) {
    1a77:	85 c0                	test   %eax,%eax
    1a79:	75 e5                	jne    1a60 <tommy_trie_inplace_remove+0x30>
		return 0;

	--trie_inplace->count;

	return ret->data;
}
    1a7b:	83 c4 04             	add    $0x4,%esp
	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];

	ret = trie_inplace_bucket_remove(TOMMY_TRIE_INPLACE_BUCKET_SHIFT, let_ptr, 0, key);

	if (!ret)
		return 0;
    1a7e:	31 c0                	xor    %eax,%eax

	--trie_inplace->count;

	return ret->data;
}
    1a80:	5b                   	pop    %ebx
    1a81:	5e                   	pop    %esi
    1a82:	5f                   	pop    %edi
    1a83:	5d                   	pop    %ebp
    1a84:	c3                   	ret    
    1a85:	8d 76 00             	lea    0x0(%esi),%esi
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
	while (node && node->key != key) {
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1a88:	8d 2c 93             	lea    (%ebx,%edx,4),%ebp
tommy_inline void tommy_trie_inplace_list_remove(tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* node)
{
	tommy_trie_inplace_node* head = *let_ptr;

	/* remove from the "circular" prev list */
	if (node->next) { 
    1a8b:	8b 10                	mov    (%eax),%edx
/**
 * Remove an element from the list.
 */
tommy_inline void tommy_trie_inplace_list_remove(tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* node)
{
	tommy_trie_inplace_node* head = *let_ptr;
    1a8d:	8b 5d 00             	mov    0x0(%ebp),%ebx

	/* remove from the "circular" prev list */
	if (node->next) { 
    1a90:	85 d2                	test   %edx,%edx
    1a92:	74 40                	je     1ad4 <tommy_trie_inplace_remove+0xa4>
		node->next->prev = node->prev;
    1a94:	8b 48 04             	mov    0x4(%eax),%ecx
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1a97:	39 c3                	cmp    %eax,%ebx
{
	tommy_trie_inplace_node* head = *let_ptr;

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
    1a99:	89 4a 04             	mov    %ecx,0x4(%edx)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1a9c:	74 42                	je     1ae0 <tommy_trie_inplace_remove+0xb0>
		*let_ptr = node->next; /* the new first */
	} else {
		node->prev->next = node->next;
    1a9e:	8b 48 04             	mov    0x4(%eax),%ecx
    1aa1:	89 11                	mov    %edx,(%ecx)
    1aa3:	8b 55 00             	mov    0x0(%ebp),%edx
	}

	tommy_trie_inplace_list_remove(let_ptr, remove);

	/* if we have a substitute */
	if (*let_ptr != 0) {
    1aa6:	85 d2                	test   %edx,%edx
    1aa8:	74 46                	je     1af0 <tommy_trie_inplace_remove+0xc0>
		/* copy the child pointers to the new one */
		node = *let_ptr;
		for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i) {
			node->map[i] = remove->map[i];
    1aaa:	8b 48 10             	mov    0x10(%eax),%ecx
    1aad:	89 4a 10             	mov    %ecx,0x10(%edx)
    1ab0:	8b 48 14             	mov    0x14(%eax),%ecx
    1ab3:	89 4a 14             	mov    %ecx,0x14(%edx)
    1ab6:	8b 48 18             	mov    0x18(%eax),%ecx
    1ab9:	89 4a 18             	mov    %ecx,0x18(%edx)
    1abc:	8b 48 1c             	mov    0x1c(%eax),%ecx
    1abf:	89 4a 1c             	mov    %ecx,0x1c(%edx)
	ret = trie_inplace_bucket_remove(TOMMY_TRIE_INPLACE_BUCKET_SHIFT, let_ptr, 0, key);

	if (!ret)
		return 0;

	--trie_inplace->count;
    1ac2:	83 ae 00 01 00 00 01 	subl   $0x1,0x100(%esi)

	return ret->data;
    1ac9:	8b 40 08             	mov    0x8(%eax),%eax
}
    1acc:	83 c4 04             	add    $0x4,%esp
    1acf:	5b                   	pop    %ebx
    1ad0:	5e                   	pop    %esi
    1ad1:	5f                   	pop    %edi
    1ad2:	5d                   	pop    %ebp
    1ad3:	c3                   	ret    

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    1ad4:	8b 48 04             	mov    0x4(%eax),%ecx
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1ad7:	39 c3                	cmp    %eax,%ebx

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    1ad9:	89 4b 04             	mov    %ecx,0x4(%ebx)
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1adc:	75 c0                	jne    1a9e <tommy_trie_inplace_remove+0x6e>
    1ade:	66 90                	xchg   %ax,%ax
	}

	tommy_trie_inplace_list_remove(let_ptr, remove);

	/* if we have a substitute */
	if (*let_ptr != 0) {
    1ae0:	85 d2                	test   %edx,%edx
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
		*let_ptr = node->next; /* the new first */
    1ae2:	89 55 00             	mov    %edx,0x0(%ebp)
	}

	tommy_trie_inplace_list_remove(let_ptr, remove);

	/* if we have a substitute */
	if (*let_ptr != 0) {
    1ae5:	75 c3                	jne    1aaa <tommy_trie_inplace_remove+0x7a>
    1ae7:	89 f6                	mov    %esi,%esi
    1ae9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
    1af0:	89 c2                	mov    %eax,%edx
    1af2:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
    1af9:	bf 03 00 00 00       	mov    $0x3,%edi
    1afe:	eb 0d                	jmp    1b0d <tommy_trie_inplace_remove+0xdd>

	/* search backward, statistically we have more zeros than ones */
	i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
	while (i >= 0) {
		if (leaf->map[i]) {
			leaf_let_ptr = &leaf->map[i];
    1b00:	8d 3c 9a             	lea    (%edx,%ebx,4),%edi
    1b03:	89 ca                	mov    %ecx,%edx
    1b05:	89 3c 24             	mov    %edi,(%esp)
			leaf = *leaf_let_ptr;
			i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
    1b08:	bf 03 00 00 00       	mov    $0x3,%edi
	leaf = remove;

	/* search backward, statistically we have more zeros than ones */
	i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
	while (i >= 0) {
		if (leaf->map[i]) {
    1b0d:	8d 5f 04             	lea    0x4(%edi),%ebx
    1b10:	8b 0c 9a             	mov    (%edx,%ebx,4),%ecx
    1b13:	85 c9                	test   %ecx,%ecx
    1b15:	75 e9                	jne    1b00 <tommy_trie_inplace_remove+0xd0>
	leaf_let_ptr = 0;
	leaf = remove;

	/* search backward, statistically we have more zeros than ones */
	i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
	while (i >= 0) {
    1b17:	83 ef 01             	sub    $0x1,%edi
    1b1a:	79 f1                	jns    1b0d <tommy_trie_inplace_remove+0xdd>
		}
		--i;
	}

	/* if it's itself a leaf */
	if (!leaf_let_ptr) {
    1b1c:	8b 3c 24             	mov    (%esp),%edi
    1b1f:	85 ff                	test   %edi,%edi
    1b21:	74 9f                	je     1ac2 <tommy_trie_inplace_remove+0x92>
		return remove;
	}

	/* remove the leaf */
	*leaf_let_ptr = 0;
    1b23:	c7 07 00 00 00 00    	movl   $0x0,(%edi)

	/* copy the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i) {
		leaf->map[i] = remove->map[i];
    1b29:	8b 48 10             	mov    0x10(%eax),%ecx
    1b2c:	89 4a 10             	mov    %ecx,0x10(%edx)
    1b2f:	8b 48 14             	mov    0x14(%eax),%ecx
    1b32:	89 4a 14             	mov    %ecx,0x14(%edx)
    1b35:	8b 48 18             	mov    0x18(%eax),%ecx
    1b38:	89 4a 18             	mov    %ecx,0x18(%edx)
    1b3b:	8b 48 1c             	mov    0x1c(%eax),%ecx
    1b3e:	89 4a 1c             	mov    %ecx,0x1c(%edx)
	}

	/* put it in place */
	*let_ptr = leaf;
    1b41:	89 55 00             	mov    %edx,0x0(%ebp)
    1b44:	e9 79 ff ff ff       	jmp    1ac2 <tommy_trie_inplace_remove+0x92>
void* tommy_trie_inplace_remove(tommy_trie_inplace* trie_inplace, tommy_key_t key)
{
	tommy_trie_inplace_node* ret;
	tommy_trie_inplace_node** let_ptr;

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1b49:	8d 2c 86             	lea    (%esi,%eax,4),%ebp
    1b4c:	89 d8                	mov    %ebx,%eax
    1b4e:	e9 38 ff ff ff       	jmp    1a8b <tommy_trie_inplace_remove+0x5b>
    1b53:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1b59:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001b60 <tommy_trie_inplace_remove_existing>:

	return ret->data;
}

void* tommy_trie_inplace_remove_existing(tommy_trie_inplace* trie_inplace, tommy_trie_inplace_node* node)
{
    1b60:	55                   	push   %ebp
    1b61:	57                   	push   %edi
    1b62:	56                   	push   %esi
    1b63:	53                   	push   %ebx
    1b64:	83 ec 1c             	sub    $0x1c,%esp
    1b67:	8b 44 24 34          	mov    0x34(%esp),%eax
    1b6b:	8b 74 24 30          	mov    0x30(%esp),%esi
	tommy_trie_inplace_node* ret;
	tommy_key_t key = node->key;
    1b6f:	8b 58 0c             	mov    0xc(%eax),%ebx
	tommy_trie_inplace_node** let_ptr;

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1b72:	89 da                	mov    %ebx,%edx
    1b74:	c1 ea 1a             	shr    $0x1a,%edx
	tommy_trie_inplace_node* node;
	int i;
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
    1b77:	8b 3c 96             	mov    (%esi,%edx,4),%edi
	while (node && node->key != key) {
    1b7a:	85 ff                	test   %edi,%edi
    1b7c:	74 2d                	je     1bab <tommy_trie_inplace_remove_existing+0x4b>
    1b7e:	3b 5f 0c             	cmp    0xc(%edi),%ebx
    1b81:	0f 84 0c 01 00 00    	je     1c93 <tommy_trie_inplace_remove_existing+0x133>
    1b87:	b9 1a 00 00 00       	mov    $0x1a,%ecx
    1b8c:	eb 09                	jmp    1b97 <tommy_trie_inplace_remove_existing+0x37>
    1b8e:	66 90                	xchg   %ax,%ax
    1b90:	3b 5d 0c             	cmp    0xc(%ebp),%ebx
    1b93:	74 33                	je     1bc8 <tommy_trie_inplace_remove_existing+0x68>
    1b95:	89 ef                	mov    %ebp,%edi
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1b97:	89 da                	mov    %ebx,%edx
    1b99:	d3 ea                	shr    %cl,%edx
		node = *let_ptr;
		shift -= TOMMY_TRIE_INPLACE_TREE_BIT;
    1b9b:	83 e9 02             	sub    $0x2,%ecx
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
	while (node && node->key != key) {
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1b9e:	83 e2 03             	and    $0x3,%edx
		node = *let_ptr;
    1ba1:	83 c2 04             	add    $0x4,%edx
    1ba4:	8b 2c 97             	mov    (%edi,%edx,4),%ebp
	int i;
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
	while (node && node->key != key) {
    1ba7:	85 ed                	test   %ebp,%ebp
    1ba9:	75 e5                	jne    1b90 <tommy_trie_inplace_remove_existing+0x30>
	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];

	ret = trie_inplace_bucket_remove(TOMMY_TRIE_INPLACE_BUCKET_SHIFT, let_ptr, node, key);

	/* the element removed must match the one passed */
	assert(ret == node);
    1bab:	68 80 00 00 00       	push   $0x80
    1bb0:	68 f5 00 00 00       	push   $0xf5
    1bb5:	68 42 00 00 00       	push   $0x42
    1bba:	68 36 00 00 00       	push   $0x36
    1bbf:	e8 fc ff ff ff       	call   1bc0 <tommy_trie_inplace_remove_existing+0x60>
    1bc4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
	tommy_trie_inplace_node** leaf_let_ptr;
	tommy_trie_inplace_node* leaf;

	node = *let_ptr;
	while (node && node->key != key) {
		let_ptr = &node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1bc8:	8d 2c 97             	lea    (%edi,%edx,4),%ebp
tommy_inline void tommy_trie_inplace_list_remove(tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* node)
{
	tommy_trie_inplace_node* head = *let_ptr;

	/* remove from the "circular" prev list */
	if (node->next) { 
    1bcb:	8b 10                	mov    (%eax),%edx
/**
 * Remove an element from the list.
 */
tommy_inline void tommy_trie_inplace_list_remove(tommy_trie_inplace_node** let_ptr, tommy_trie_inplace_node* node)
{
	tommy_trie_inplace_node* head = *let_ptr;
    1bcd:	8b 5d 00             	mov    0x0(%ebp),%ebx

	/* remove from the "circular" prev list */
	if (node->next) { 
    1bd0:	85 d2                	test   %edx,%edx
    1bd2:	0f 84 b0 00 00 00    	je     1c88 <tommy_trie_inplace_remove_existing+0x128>
		node->next->prev = node->prev;
    1bd8:	8b 48 04             	mov    0x4(%eax),%ecx
    1bdb:	89 4a 04             	mov    %ecx,0x4(%edx)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1bde:	39 d8                	cmp    %ebx,%eax
    1be0:	0f 84 9a 00 00 00    	je     1c80 <tommy_trie_inplace_remove_existing+0x120>
		*let_ptr = node->next; /* the new first */
	} else {
		node->prev->next = node->next;
    1be6:	8b 48 04             	mov    0x4(%eax),%ecx
    1be9:	89 11                	mov    %edx,(%ecx)
    1beb:	8b 55 00             	mov    0x0(%ebp),%edx
	}

	tommy_trie_inplace_list_remove(let_ptr, remove);

	/* if we have a substitute */
	if (*let_ptr != 0) {
    1bee:	85 d2                	test   %edx,%edx
    1bf0:	74 2e                	je     1c20 <tommy_trie_inplace_remove_existing+0xc0>
		/* copy the child pointers to the new one */
		node = *let_ptr;
		for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i) {
			node->map[i] = remove->map[i];
    1bf2:	8b 48 10             	mov    0x10(%eax),%ecx
    1bf5:	89 4a 10             	mov    %ecx,0x10(%edx)
    1bf8:	8b 48 14             	mov    0x14(%eax),%ecx
    1bfb:	89 4a 14             	mov    %ecx,0x14(%edx)
    1bfe:	8b 48 18             	mov    0x18(%eax),%ecx
    1c01:	89 4a 18             	mov    %ecx,0x18(%edx)
    1c04:	8b 48 1c             	mov    0x1c(%eax),%ecx
    1c07:	89 4a 1c             	mov    %ecx,0x1c(%edx)
	ret = trie_inplace_bucket_remove(TOMMY_TRIE_INPLACE_BUCKET_SHIFT, let_ptr, node, key);

	/* the element removed must match the one passed */
	assert(ret == node);

	--trie_inplace->count;
    1c0a:	83 ae 00 01 00 00 01 	subl   $0x1,0x100(%esi)

	return ret->data;
    1c11:	8b 40 08             	mov    0x8(%eax),%eax
}
    1c14:	83 c4 1c             	add    $0x1c,%esp
    1c17:	5b                   	pop    %ebx
    1c18:	5e                   	pop    %esi
    1c19:	5f                   	pop    %edi
    1c1a:	5d                   	pop    %ebp
    1c1b:	c3                   	ret    
    1c1c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
	}

	tommy_trie_inplace_list_remove(let_ptr, remove);

	/* if we have a substitute */
	if (*let_ptr != 0) {
    1c20:	89 c2                	mov    %eax,%edx
    1c22:	c7 44 24 0c 00 00 00 	movl   $0x0,0xc(%esp)
    1c29:	00 
    1c2a:	bf 03 00 00 00       	mov    $0x3,%edi
    1c2f:	eb 15                	jmp    1c46 <tommy_trie_inplace_remove_existing+0xe6>
    1c31:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

	/* search backward, statistically we have more zeros than ones */
	i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
	while (i >= 0) {
		if (leaf->map[i]) {
			leaf_let_ptr = &leaf->map[i];
    1c38:	8d 3c 9a             	lea    (%edx,%ebx,4),%edi
    1c3b:	89 ca                	mov    %ecx,%edx
    1c3d:	89 7c 24 0c          	mov    %edi,0xc(%esp)
			leaf = *leaf_let_ptr;
			i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
    1c41:	bf 03 00 00 00       	mov    $0x3,%edi
	leaf = remove;

	/* search backward, statistically we have more zeros than ones */
	i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
	while (i >= 0) {
		if (leaf->map[i]) {
    1c46:	8d 5f 04             	lea    0x4(%edi),%ebx
    1c49:	8b 0c 9a             	mov    (%edx,%ebx,4),%ecx
    1c4c:	85 c9                	test   %ecx,%ecx
    1c4e:	75 e8                	jne    1c38 <tommy_trie_inplace_remove_existing+0xd8>
	leaf_let_ptr = 0;
	leaf = remove;

	/* search backward, statistically we have more zeros than ones */
	i = TOMMY_TRIE_INPLACE_TREE_MAX-1;
	while (i >= 0) {
    1c50:	83 ef 01             	sub    $0x1,%edi
    1c53:	79 f1                	jns    1c46 <tommy_trie_inplace_remove_existing+0xe6>
		}
		--i;
	}

	/* if it's itself a leaf */
	if (!leaf_let_ptr) {
    1c55:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
    1c59:	85 db                	test   %ebx,%ebx
    1c5b:	74 ad                	je     1c0a <tommy_trie_inplace_remove_existing+0xaa>
		return remove;
	}

	/* remove the leaf */
	*leaf_let_ptr = 0;
    1c5d:	c7 03 00 00 00 00    	movl   $0x0,(%ebx)

	/* copy the child pointers */
	for(i=0;i<TOMMY_TRIE_INPLACE_TREE_MAX;++i) {
		leaf->map[i] = remove->map[i];
    1c63:	8b 48 10             	mov    0x10(%eax),%ecx
    1c66:	89 4a 10             	mov    %ecx,0x10(%edx)
    1c69:	8b 48 14             	mov    0x14(%eax),%ecx
    1c6c:	89 4a 14             	mov    %ecx,0x14(%edx)
    1c6f:	8b 48 18             	mov    0x18(%eax),%ecx
    1c72:	89 4a 18             	mov    %ecx,0x18(%edx)
    1c75:	8b 48 1c             	mov    0x1c(%eax),%ecx
    1c78:	89 4a 1c             	mov    %ecx,0x1c(%edx)
	}

	/* put it in place */
	*let_ptr = leaf;
    1c7b:	89 55 00             	mov    %edx,0x0(%ebp)
    1c7e:	eb 8a                	jmp    1c0a <tommy_trie_inplace_remove_existing+0xaa>
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
		*let_ptr = node->next; /* the new first */
    1c80:	89 55 00             	mov    %edx,0x0(%ebp)
    1c83:	e9 66 ff ff ff       	jmp    1bee <tommy_trie_inplace_remove_existing+0x8e>

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    1c88:	8b 48 04             	mov    0x4(%eax),%ecx
    1c8b:	89 4b 04             	mov    %ecx,0x4(%ebx)
    1c8e:	e9 4b ff ff ff       	jmp    1bde <tommy_trie_inplace_remove_existing+0x7e>
{
	tommy_trie_inplace_node* ret;
	tommy_key_t key = node->key;
	tommy_trie_inplace_node** let_ptr;

	let_ptr = &trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1c93:	8d 2c 96             	lea    (%esi,%edx,4),%ebp
    1c96:	e9 30 ff ff ff       	jmp    1bcb <tommy_trie_inplace_remove_existing+0x6b>
    1c9b:	90                   	nop
    1c9c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00001ca0 <tommy_trie_inplace_bucket>:

	return ret->data;
}

tommy_trie_inplace_node* tommy_trie_inplace_bucket(tommy_trie_inplace* trie_inplace, tommy_key_t key)
{
    1ca0:	53                   	push   %ebx
    1ca1:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
	tommy_trie_inplace_node* node;
	unsigned shift;

	node = trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
    1ca5:	8b 44 24 08          	mov    0x8(%esp),%eax
    1ca9:	89 da                	mov    %ebx,%edx
    1cab:	c1 ea 1a             	shr    $0x1a,%edx
    1cae:	8b 04 90             	mov    (%eax,%edx,4),%eax
	shift = TOMMY_TRIE_INPLACE_BUCKET_SHIFT;

	while (node && node->key != key) {
    1cb1:	85 c0                	test   %eax,%eax
    1cb3:	74 2a                	je     1cdf <tommy_trie_inplace_bucket+0x3f>
    1cb5:	39 58 0c             	cmp    %ebx,0xc(%eax)
    1cb8:	74 25                	je     1cdf <tommy_trie_inplace_bucket+0x3f>
    1cba:	b9 1a 00 00 00       	mov    $0x1a,%ecx
    1cbf:	eb 0c                	jmp    1ccd <tommy_trie_inplace_bucket+0x2d>
    1cc1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    1cc8:	39 58 0c             	cmp    %ebx,0xc(%eax)
    1ccb:	74 12                	je     1cdf <tommy_trie_inplace_bucket+0x3f>
		node = node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1ccd:	89 da                	mov    %ebx,%edx
    1ccf:	d3 ea                	shr    %cl,%edx
		shift -= TOMMY_TRIE_INPLACE_TREE_BIT;
    1cd1:	83 e9 02             	sub    $0x2,%ecx

	node = trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
	shift = TOMMY_TRIE_INPLACE_BUCKET_SHIFT;

	while (node && node->key != key) {
		node = node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
    1cd4:	83 e2 03             	and    $0x3,%edx
    1cd7:	8b 44 90 10          	mov    0x10(%eax,%edx,4),%eax
	unsigned shift;

	node = trie_inplace->bucket[key >> TOMMY_TRIE_INPLACE_BUCKET_SHIFT];
	shift = TOMMY_TRIE_INPLACE_BUCKET_SHIFT;

	while (node && node->key != key) {
    1cdb:	85 c0                	test   %eax,%eax
    1cdd:	75 e9                	jne    1cc8 <tommy_trie_inplace_bucket+0x28>
		node = node->map[(key >> shift) & TOMMY_TRIE_INPLACE_TREE_MASK];
		shift -= TOMMY_TRIE_INPLACE_TREE_BIT;
	}

	return node;
}
    1cdf:	5b                   	pop    %ebx
    1ce0:	c3                   	ret    
    1ce1:	eb 0d                	jmp    1cf0 <tommy_trie_inplace_memory_usage>
    1ce3:	90                   	nop
    1ce4:	90                   	nop
    1ce5:	90                   	nop
    1ce6:	90                   	nop
    1ce7:	90                   	nop
    1ce8:	90                   	nop
    1ce9:	90                   	nop
    1cea:	90                   	nop
    1ceb:	90                   	nop
    1cec:	90                   	nop
    1ced:	90                   	nop
    1cee:	90                   	nop
    1cef:	90                   	nop

00001cf0 <tommy_trie_inplace_memory_usage>:

tommy_size_t tommy_trie_inplace_memory_usage(tommy_trie_inplace* trie_inplace)
{
	return tommy_trie_inplace_count(trie_inplace) * (tommy_size_t)sizeof(tommy_trie_inplace_node);
    1cf0:	8b 44 24 04          	mov    0x4(%esp),%eax
    1cf4:	8b 80 00 01 00 00    	mov    0x100(%eax),%eax
    1cfa:	c1 e0 05             	shl    $0x5,%eax
}
    1cfd:	c3                   	ret    
    1cfe:	66 90                	xchg   %ax,%ax

00001d00 <tommy_hashtable_init>:

/******************************************************************************/
/* hashtable */

void tommy_hashtable_init(tommy_hashtable* hashtable, unsigned bucket_max)
{
    1d00:	56                   	push   %esi
    1d01:	53                   	push   %ebx
    1d02:	83 ec 04             	sub    $0x4,%esp
    1d05:	8b 44 24 14          	mov    0x14(%esp),%eax
    1d09:	8b 5c 24 10          	mov    0x10(%esp),%ebx
	if (bucket_max < 16) {
    1d0d:	83 f8 0f             	cmp    $0xf,%eax
    1d10:	76 5e                	jbe    1d70 <tommy_hashtable_init+0x70>
tommy_inline tommy_uint32_t tommy_roundup_pow2_u32(tommy_uint32_t value)
{
	/* Round up to the next highest power of 2 */
	/* from http://www-graphics.stanford.edu/~seander/bithacks.html */

	--value;
    1d12:	83 e8 01             	sub    $0x1,%eax
	value |= value >> 1;
    1d15:	89 c2                	mov    %eax,%edx
    1d17:	d1 ea                	shr    %edx
    1d19:	09 d0                	or     %edx,%eax
	value |= value >> 2;
    1d1b:	89 c2                	mov    %eax,%edx
    1d1d:	c1 ea 02             	shr    $0x2,%edx
    1d20:	09 d0                	or     %edx,%eax
	value |= value >> 4;
    1d22:	89 c2                	mov    %eax,%edx
    1d24:	c1 ea 04             	shr    $0x4,%edx
    1d27:	09 d0                	or     %edx,%eax
	value |= value >> 8;
    1d29:	89 c2                	mov    %eax,%edx
    1d2b:	c1 ea 08             	shr    $0x8,%edx
    1d2e:	09 d0                	or     %edx,%eax
	value |= value >> 16;
    1d30:	89 c2                	mov    %eax,%edx
    1d32:	c1 ea 10             	shr    $0x10,%edx
    1d35:	09 d0                	or     %edx,%eax
	++value;
    1d37:	8d 50 01             	lea    0x1(%eax),%edx
    1d3a:	8d 34 95 00 00 00 00 	lea    0x0(,%edx,4),%esi
		bucket_max = tommy_roundup_pow2_u32(bucket_max);
	}

	hashtable->bucket_max = bucket_max;
	hashtable->bucket_mask = hashtable->bucket_max - 1;
	hashtable->bucket = tommy_cast(tommy_hashtable_node**, tommy_malloc(hashtable->bucket_max * sizeof(tommy_hashtable_node*)));
    1d41:	83 ec 0c             	sub    $0xc,%esp
		bucket_max = 16;
	} else {
		bucket_max = tommy_roundup_pow2_u32(bucket_max);
	}

	hashtable->bucket_max = bucket_max;
    1d44:	89 53 04             	mov    %edx,0x4(%ebx)
	hashtable->bucket_mask = hashtable->bucket_max - 1;
    1d47:	89 43 08             	mov    %eax,0x8(%ebx)
	hashtable->bucket = tommy_cast(tommy_hashtable_node**, tommy_malloc(hashtable->bucket_max * sizeof(tommy_hashtable_node*)));
    1d4a:	56                   	push   %esi
    1d4b:	e8 fc ff ff ff       	call   1d4c <tommy_hashtable_init+0x4c>
	memset(hashtable->bucket, 0, hashtable->bucket_max * sizeof(tommy_hashtable_node*));
    1d50:	83 c4 0c             	add    $0xc,%esp
		bucket_max = tommy_roundup_pow2_u32(bucket_max);
	}

	hashtable->bucket_max = bucket_max;
	hashtable->bucket_mask = hashtable->bucket_max - 1;
	hashtable->bucket = tommy_cast(tommy_hashtable_node**, tommy_malloc(hashtable->bucket_max * sizeof(tommy_hashtable_node*)));
    1d53:	89 03                	mov    %eax,(%ebx)
	memset(hashtable->bucket, 0, hashtable->bucket_max * sizeof(tommy_hashtable_node*));
    1d55:	56                   	push   %esi
    1d56:	6a 00                	push   $0x0
    1d58:	50                   	push   %eax
    1d59:	e8 fc ff ff ff       	call   1d5a <tommy_hashtable_init+0x5a>

	hashtable->count = 0;
    1d5e:	c7 43 0c 00 00 00 00 	movl   $0x0,0xc(%ebx)
}
    1d65:	83 c4 14             	add    $0x14,%esp
    1d68:	5b                   	pop    %ebx
    1d69:	5e                   	pop    %esi
    1d6a:	c3                   	ret    
    1d6b:	90                   	nop
    1d6c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1d70:	be 40 00 00 00       	mov    $0x40,%esi
    1d75:	b8 0f 00 00 00       	mov    $0xf,%eax
/* hashtable */

void tommy_hashtable_init(tommy_hashtable* hashtable, unsigned bucket_max)
{
	if (bucket_max < 16) {
		bucket_max = 16;
    1d7a:	ba 10 00 00 00       	mov    $0x10,%edx
    1d7f:	eb c0                	jmp    1d41 <tommy_hashtable_init+0x41>
    1d81:	eb 0d                	jmp    1d90 <tommy_hashtable_done>
    1d83:	90                   	nop
    1d84:	90                   	nop
    1d85:	90                   	nop
    1d86:	90                   	nop
    1d87:	90                   	nop
    1d88:	90                   	nop
    1d89:	90                   	nop
    1d8a:	90                   	nop
    1d8b:	90                   	nop
    1d8c:	90                   	nop
    1d8d:	90                   	nop
    1d8e:	90                   	nop
    1d8f:	90                   	nop

00001d90 <tommy_hashtable_done>:
	hashtable->count = 0;
}

void tommy_hashtable_done(tommy_hashtable* hashtable)
{
	tommy_free(hashtable->bucket);
    1d90:	8b 44 24 04          	mov    0x4(%esp),%eax
    1d94:	8b 00                	mov    (%eax),%eax
    1d96:	89 44 24 04          	mov    %eax,0x4(%esp)
    1d9a:	e9 fc ff ff ff       	jmp    1d9b <tommy_hashtable_done+0xb>
    1d9f:	90                   	nop

00001da0 <tommy_hashtable_insert>:
}

void tommy_hashtable_insert(tommy_hashtable* hashtable, tommy_hashtable_node* node, void* data, tommy_hash_t hash)
{
    1da0:	56                   	push   %esi
    1da1:	53                   	push   %ebx
    1da2:	8b 54 24 0c          	mov    0xc(%esp),%edx
    1da6:	8b 5c 24 18          	mov    0x18(%esp),%ebx
    1daa:	8b 44 24 10          	mov    0x10(%esp),%eax
	unsigned pos = hash & hashtable->bucket_mask;
    1dae:	8b 72 08             	mov    0x8(%edx),%esi

	tommy_list_insert_tail(&hashtable->bucket[pos], node, data);
    1db1:	8b 0a                	mov    (%edx),%ecx
	tommy_free(hashtable->bucket);
}

void tommy_hashtable_insert(tommy_hashtable* hashtable, tommy_hashtable_node* node, void* data, tommy_hash_t hash)
{
	unsigned pos = hash & hashtable->bucket_mask;
    1db3:	21 de                	and    %ebx,%esi

	tommy_list_insert_tail(&hashtable->bucket[pos], node, data);
    1db5:	8d 34 b1             	lea    (%ecx,%esi,4),%esi
    1db8:	8b 0e                	mov    (%esi),%ecx
 */
tommy_inline void tommy_list_insert_tail(tommy_list* list, tommy_node* node, void* data)
{
	tommy_node* head = tommy_list_head(list);

	if (head) {
    1dba:	85 c9                	test   %ecx,%ecx
    1dbc:	74 2a                	je     1de8 <tommy_hashtable_insert+0x48>
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_tail_not_empty(tommy_node* head, tommy_node* node)
{
	/* insert in the "circular" prev list */
	node->prev = head->prev;
    1dbe:	8b 71 04             	mov    0x4(%ecx),%esi
    1dc1:	89 70 04             	mov    %esi,0x4(%eax)
	head->prev = node;
    1dc4:	89 41 04             	mov    %eax,0x4(%ecx)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
    1dc7:	8b 48 04             	mov    0x4(%eax),%ecx
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
    1dca:	c7 00 00 00 00 00    	movl   $0x0,(%eax)
	node->prev->next = node;
    1dd0:	89 01                	mov    %eax,(%ecx)
		tommy_list_insert_tail_not_empty(head, node);
	} else {
		tommy_list_insert_first(list, node);
	}

	node->data = data;
    1dd2:	8b 4c 24 14          	mov    0x14(%esp),%ecx

	node->key = hash;
    1dd6:	89 58 0c             	mov    %ebx,0xc(%eax)
    1dd9:	89 48 08             	mov    %ecx,0x8(%eax)

	++hashtable->count;
    1ddc:	83 42 0c 01          	addl   $0x1,0xc(%edx)
}
    1de0:	5b                   	pop    %ebx
    1de1:	5e                   	pop    %esi
    1de2:	c3                   	ret    
    1de3:	90                   	nop
    1de4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    1de8:	89 40 04             	mov    %eax,0x4(%eax)

	/* one element "0 terminated" next list */
	node->next = 0;
    1deb:	c7 00 00 00 00 00    	movl   $0x0,(%eax)

	*list = node;
    1df1:	89 06                	mov    %eax,(%esi)
    1df3:	eb dd                	jmp    1dd2 <tommy_hashtable_insert+0x32>
    1df5:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1df9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001e00 <tommy_hashtable_remove_existing>:

void* tommy_hashtable_remove_existing(tommy_hashtable* hashtable, tommy_hashtable_node* node)
{
    1e00:	57                   	push   %edi
    1e01:	56                   	push   %esi
    1e02:	53                   	push   %ebx
    1e03:	8b 44 24 14          	mov    0x14(%esp),%eax
    1e07:	8b 54 24 10          	mov    0x10(%esp),%edx
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
    1e0b:	8b 78 04             	mov    0x4(%eax),%edi
	unsigned pos = node->key & hashtable->bucket_mask;
    1e0e:	8b 4a 08             	mov    0x8(%edx),%ecx
    1e11:	23 48 0c             	and    0xc(%eax),%ecx

	tommy_list_remove_existing(&hashtable->bucket[pos], node);
    1e14:	8b 1a                	mov    (%edx),%ebx
    1e16:	8d 34 8b             	lea    (%ebx,%ecx,4),%esi
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    1e19:	8b 08                	mov    (%eax),%ecx
    1e1b:	8b 1e                	mov    (%esi),%ebx
    1e1d:	85 c9                	test   %ecx,%ecx
    1e1f:	74 1f                	je     1e40 <tommy_hashtable_remove_existing+0x40>
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1e21:	39 d8                	cmp    %ebx,%eax
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
    1e23:	89 79 04             	mov    %edi,0x4(%ecx)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1e26:	74 28                	je     1e50 <tommy_hashtable_remove_existing+0x50>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    1e28:	8b 58 04             	mov    0x4(%eax),%ebx

	--hashtable->count;

	return node->data;
    1e2b:	8b 40 08             	mov    0x8(%eax),%eax
    1e2e:	89 0b                	mov    %ecx,(%ebx)
{
	unsigned pos = node->key & hashtable->bucket_mask;

	tommy_list_remove_existing(&hashtable->bucket[pos], node);

	--hashtable->count;
    1e30:	83 6a 0c 01          	subl   $0x1,0xc(%edx)

	return node->data;
}
    1e34:	5b                   	pop    %ebx
    1e35:	5e                   	pop    %esi
    1e36:	5f                   	pop    %edi
    1e37:	c3                   	ret    
    1e38:	90                   	nop
    1e39:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1e40:	39 d8                	cmp    %ebx,%eax

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    1e42:	89 7b 04             	mov    %edi,0x4(%ebx)
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1e45:	75 e1                	jne    1e28 <tommy_hashtable_remove_existing+0x28>
    1e47:	89 f6                	mov    %esi,%esi
    1e49:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
		*list = node->next; /* the new head, in case 0 */
    1e50:	89 0e                	mov    %ecx,(%esi)
{
	unsigned pos = node->key & hashtable->bucket_mask;

	tommy_list_remove_existing(&hashtable->bucket[pos], node);

	--hashtable->count;
    1e52:	83 6a 0c 01          	subl   $0x1,0xc(%edx)

	return node->data;
}
    1e56:	5b                   	pop    %ebx

	tommy_list_remove_existing(&hashtable->bucket[pos], node);

	--hashtable->count;

	return node->data;
    1e57:	8b 40 08             	mov    0x8(%eax),%eax
}
    1e5a:	5e                   	pop    %esi
    1e5b:	5f                   	pop    %edi
    1e5c:	c3                   	ret    
    1e5d:	8d 76 00             	lea    0x0(%esi),%esi

00001e60 <tommy_hashtable_remove>:

void* tommy_hashtable_remove(tommy_hashtable* hashtable, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
    1e60:	55                   	push   %ebp
    1e61:	57                   	push   %edi
    1e62:	56                   	push   %esi
    1e63:	53                   	push   %ebx
    1e64:	83 ec 1c             	sub    $0x1c,%esp
    1e67:	8b 7c 24 30          	mov    0x30(%esp),%edi
    1e6b:	8b 5c 24 3c          	mov    0x3c(%esp),%ebx
    1e6f:	8b 74 24 34          	mov    0x34(%esp),%esi
	unsigned pos = hash & hashtable->bucket_mask;
    1e73:	8b 47 08             	mov    0x8(%edi),%eax
	tommy_hashtable_node* i = hashtable->bucket[pos];
    1e76:	8b 17                	mov    (%edi),%edx
	return node->data;
}

void* tommy_hashtable_remove(tommy_hashtable* hashtable, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
	unsigned pos = hash & hashtable->bucket_mask;
    1e78:	21 d8                	and    %ebx,%eax
	tommy_hashtable_node* i = hashtable->bucket[pos];
    1e7a:	8b 2c 82             	mov    (%edx,%eax,4),%ebp
    1e7d:	8d 0c 85 00 00 00 00 	lea    0x0(,%eax,4),%ecx
    1e84:	89 4c 24 0c          	mov    %ecx,0xc(%esp)

	while (i) {
    1e88:	85 ed                	test   %ebp,%ebp
    1e8a:	75 0b                	jne    1e97 <tommy_hashtable_remove+0x37>
    1e8c:	eb 52                	jmp    1ee0 <tommy_hashtable_remove+0x80>
    1e8e:	66 90                	xchg   %ax,%ax
			--hashtable->count;

			return i->data;
		}
		/* we assume that i->next is still valid also after removing */
		i = i->next;
    1e90:	8b 6d 00             	mov    0x0(%ebp),%ebp
void* tommy_hashtable_remove(tommy_hashtable* hashtable, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
	unsigned pos = hash & hashtable->bucket_mask;
	tommy_hashtable_node* i = hashtable->bucket[pos];

	while (i) {
    1e93:	85 ed                	test   %ebp,%ebp
    1e95:	74 49                	je     1ee0 <tommy_hashtable_remove+0x80>
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
    1e97:	39 5d 0c             	cmp    %ebx,0xc(%ebp)
    1e9a:	75 f4                	jne    1e90 <tommy_hashtable_remove+0x30>
    1e9c:	83 ec 08             	sub    $0x8,%esp
    1e9f:	ff 75 08             	pushl  0x8(%ebp)
    1ea2:	ff 74 24 44          	pushl  0x44(%esp)
    1ea6:	ff d6                	call   *%esi
    1ea8:	83 c4 10             	add    $0x10,%esp
    1eab:	85 c0                	test   %eax,%eax
    1ead:	75 e1                	jne    1e90 <tommy_hashtable_remove+0x30>
			tommy_list_remove_existing(&hashtable->bucket[pos], i);
    1eaf:	8b 54 24 0c          	mov    0xc(%esp),%edx
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    1eb3:	8b 45 00             	mov    0x0(%ebp),%eax
    1eb6:	03 17                	add    (%edi),%edx
		node->next->prev = node->prev;
    1eb8:	8b 5d 04             	mov    0x4(%ebp),%ebx
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    1ebb:	85 c0                	test   %eax,%eax
    1ebd:	8b 0a                	mov    (%edx),%ecx
    1ebf:	74 29                	je     1eea <tommy_hashtable_remove+0x8a>
		node->next->prev = node->prev;
    1ec1:	89 58 04             	mov    %ebx,0x4(%eax)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    1ec4:	39 e9                	cmp    %ebp,%ecx
    1ec6:	74 27                	je     1eef <tommy_hashtable_remove+0x8f>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    1ec8:	8b 4d 04             	mov    0x4(%ebp),%ecx
    1ecb:	89 01                	mov    %eax,(%ecx)

			--hashtable->count;
    1ecd:	83 6f 0c 01          	subl   $0x1,0xc(%edi)

			return i->data;
    1ed1:	8b 45 08             	mov    0x8(%ebp),%eax
		/* we assume that i->next is still valid also after removing */
		i = i->next;
	}

	return 0;
}
    1ed4:	83 c4 1c             	add    $0x1c,%esp
    1ed7:	5b                   	pop    %ebx
    1ed8:	5e                   	pop    %esi
    1ed9:	5f                   	pop    %edi
    1eda:	5d                   	pop    %ebp
    1edb:	c3                   	ret    
    1edc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    1ee0:	83 c4 1c             	add    $0x1c,%esp
		}
		/* we assume that i->next is still valid also after removing */
		i = i->next;
	}

	return 0;
    1ee3:	31 c0                	xor    %eax,%eax
}
    1ee5:	5b                   	pop    %ebx
    1ee6:	5e                   	pop    %esi
    1ee7:	5f                   	pop    %edi
    1ee8:	5d                   	pop    %ebp
    1ee9:	c3                   	ret    

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    1eea:	89 59 04             	mov    %ebx,0x4(%ecx)
    1eed:	eb d5                	jmp    1ec4 <tommy_hashtable_remove+0x64>
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
		*list = node->next; /* the new head, in case 0 */
    1eef:	89 02                	mov    %eax,(%edx)
    1ef1:	eb da                	jmp    1ecd <tommy_hashtable_remove+0x6d>
    1ef3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    1ef9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00001f00 <tommy_hashtable_memory_usage>:

tommy_size_t tommy_hashtable_memory_usage(tommy_hashtable* hashtable)
{
    1f00:	8b 44 24 04          	mov    0x4(%esp),%eax
	return hashtable->bucket_max * (tommy_size_t)sizeof(hashtable->bucket[0])
		+ tommy_hashtable_count(hashtable) * (tommy_size_t)sizeof(tommy_hashtable_node);
    1f04:	8b 50 0c             	mov    0xc(%eax),%edx
    1f07:	8b 40 04             	mov    0x4(%eax),%eax
    1f0a:	8d 04 90             	lea    (%eax,%edx,4),%eax
	return 0;
}

tommy_size_t tommy_hashtable_memory_usage(tommy_hashtable* hashtable)
{
	return hashtable->bucket_max * (tommy_size_t)sizeof(hashtable->bucket[0])
    1f0d:	c1 e0 02             	shl    $0x2,%eax
		+ tommy_hashtable_count(hashtable) * (tommy_size_t)sizeof(tommy_hashtable_node);
}
    1f10:	c3                   	ret    
    1f11:	eb 0d                	jmp    1f20 <tommy_hashdyn_init>
    1f13:	90                   	nop
    1f14:	90                   	nop
    1f15:	90                   	nop
    1f16:	90                   	nop
    1f17:	90                   	nop
    1f18:	90                   	nop
    1f19:	90                   	nop
    1f1a:	90                   	nop
    1f1b:	90                   	nop
    1f1c:	90                   	nop
    1f1d:	90                   	nop
    1f1e:	90                   	nop
    1f1f:	90                   	nop

00001f20 <tommy_hashdyn_init>:

/******************************************************************************/
/* hashdyn */

void tommy_hashdyn_init(tommy_hashdyn* hashdyn)
{
    1f20:	57                   	push   %edi
    1f21:	53                   	push   %ebx
    1f22:	83 ec 10             	sub    $0x10,%esp
    1f25:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
	/* fixed initial size */
	hashdyn->bucket_bit = TOMMY_HASHDYN_BIT;
    1f29:	c7 43 04 04 00 00 00 	movl   $0x4,0x4(%ebx)
	hashdyn->bucket_max = 1 << hashdyn->bucket_bit;
    1f30:	c7 43 08 10 00 00 00 	movl   $0x10,0x8(%ebx)
	hashdyn->bucket_mask = hashdyn->bucket_max - 1;
    1f37:	c7 43 0c 0f 00 00 00 	movl   $0xf,0xc(%ebx)
	hashdyn->bucket = tommy_cast(tommy_hashdyn_node**, tommy_malloc(hashdyn->bucket_max * sizeof(tommy_hashdyn_node*)));
    1f3e:	6a 40                	push   $0x40
    1f40:	e8 fc ff ff ff       	call   1f41 <tommy_hashdyn_init+0x21>
    1f45:	89 c2                	mov    %eax,%edx
    1f47:	89 03                	mov    %eax,(%ebx)
	memset(hashdyn->bucket, 0, hashdyn->bucket_max * sizeof(tommy_hashdyn_node*));
    1f49:	b9 10 00 00 00       	mov    $0x10,%ecx
    1f4e:	31 c0                	xor    %eax,%eax
    1f50:	89 d7                	mov    %edx,%edi

	hashdyn->count = 0;
    1f52:	c7 43 10 00 00 00 00 	movl   $0x0,0x10(%ebx)
	/* fixed initial size */
	hashdyn->bucket_bit = TOMMY_HASHDYN_BIT;
	hashdyn->bucket_max = 1 << hashdyn->bucket_bit;
	hashdyn->bucket_mask = hashdyn->bucket_max - 1;
	hashdyn->bucket = tommy_cast(tommy_hashdyn_node**, tommy_malloc(hashdyn->bucket_max * sizeof(tommy_hashdyn_node*)));
	memset(hashdyn->bucket, 0, hashdyn->bucket_max * sizeof(tommy_hashdyn_node*));
    1f59:	f3 ab                	rep stos %eax,%es:(%edi)

	hashdyn->count = 0;
}
    1f5b:	83 c4 14             	add    $0x14,%esp
    1f5e:	5b                   	pop    %ebx
    1f5f:	5f                   	pop    %edi
    1f60:	c3                   	ret    
    1f61:	eb 0d                	jmp    1f70 <tommy_hashdyn_done>
    1f63:	90                   	nop
    1f64:	90                   	nop
    1f65:	90                   	nop
    1f66:	90                   	nop
    1f67:	90                   	nop
    1f68:	90                   	nop
    1f69:	90                   	nop
    1f6a:	90                   	nop
    1f6b:	90                   	nop
    1f6c:	90                   	nop
    1f6d:	90                   	nop
    1f6e:	90                   	nop
    1f6f:	90                   	nop

00001f70 <tommy_hashdyn_done>:

void tommy_hashdyn_done(tommy_hashdyn* hashdyn)
{
	tommy_free(hashdyn->bucket);
    1f70:	8b 44 24 04          	mov    0x4(%esp),%eax
    1f74:	8b 00                	mov    (%eax),%eax
    1f76:	89 44 24 04          	mov    %eax,0x4(%esp)
    1f7a:	e9 fc ff ff ff       	jmp    1f7b <tommy_hashdyn_done+0xb>
    1f7f:	90                   	nop

00001f80 <tommy_hashdyn_insert>:
		tommy_hashdyn_resize(hashdyn, hashdyn->bucket_bit - 1);
	}
}

void tommy_hashdyn_insert(tommy_hashdyn* hashdyn, tommy_hashdyn_node* node, void* data, tommy_hash_t hash)
{
    1f80:	57                   	push   %edi
    1f81:	56                   	push   %esi
    1f82:	53                   	push   %ebx
    1f83:	8b 44 24 10          	mov    0x10(%esp),%eax
    1f87:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
    1f8b:	8b 54 24 14          	mov    0x14(%esp),%edx
    1f8f:	8b 7c 24 18          	mov    0x18(%esp),%edi
	unsigned pos = hash & hashdyn->bucket_mask;
    1f93:	8b 70 0c             	mov    0xc(%eax),%esi

	tommy_list_insert_tail(&hashdyn->bucket[pos], node, data);
    1f96:	8b 08                	mov    (%eax),%ecx
	}
}

void tommy_hashdyn_insert(tommy_hashdyn* hashdyn, tommy_hashdyn_node* node, void* data, tommy_hash_t hash)
{
	unsigned pos = hash & hashdyn->bucket_mask;
    1f98:	21 de                	and    %ebx,%esi

	tommy_list_insert_tail(&hashdyn->bucket[pos], node, data);
    1f9a:	8d 34 b1             	lea    (%ecx,%esi,4),%esi
    1f9d:	8b 0e                	mov    (%esi),%ecx
 */
tommy_inline void tommy_list_insert_tail(tommy_list* list, tommy_node* node, void* data)
{
	tommy_node* head = tommy_list_head(list);

	if (head) {
    1f9f:	85 c9                	test   %ecx,%ecx
    1fa1:	74 4d                	je     1ff0 <tommy_hashdyn_insert+0x70>
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_tail_not_empty(tommy_node* head, tommy_node* node)
{
	/* insert in the "circular" prev list */
	node->prev = head->prev;
    1fa3:	8b 71 04             	mov    0x4(%ecx),%esi
    1fa6:	89 72 04             	mov    %esi,0x4(%edx)
	head->prev = node;
    1fa9:	89 51 04             	mov    %edx,0x4(%ecx)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
    1fac:	8b 4a 04             	mov    0x4(%edx),%ecx
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
    1faf:	c7 02 00 00 00 00    	movl   $0x0,(%edx)
	node->prev->next = node;
    1fb5:	89 11                	mov    %edx,(%ecx)
		tommy_list_insert_tail_not_empty(head, node);
	} else {
		tommy_list_insert_first(list, node);
	}

	node->data = data;
    1fb7:	89 7a 08             	mov    %edi,0x8(%edx)

	node->key = hash;

	++hashdyn->count;
    1fba:	8b 78 10             	mov    0x10(%eax),%edi
 * Grow.
 */
tommy_inline void hashdyn_grow_step(tommy_hashdyn* hashdyn)
{
	/* grow if more than 50% full */
	if (hashdyn->count >= hashdyn->bucket_max / 2) {
    1fbd:	8b 48 08             	mov    0x8(%eax),%ecx
{
	unsigned pos = hash & hashdyn->bucket_mask;

	tommy_list_insert_tail(&hashdyn->bucket[pos], node, data);

	node->key = hash;
    1fc0:	89 5a 0c             	mov    %ebx,0xc(%edx)

	++hashdyn->count;
    1fc3:	8d 57 01             	lea    0x1(%edi),%edx
 * Grow.
 */
tommy_inline void hashdyn_grow_step(tommy_hashdyn* hashdyn)
{
	/* grow if more than 50% full */
	if (hashdyn->count >= hashdyn->bucket_max / 2) {
    1fc6:	d1 e9                	shr    %ecx
    1fc8:	39 ca                	cmp    %ecx,%edx

	tommy_list_insert_tail(&hashdyn->bucket[pos], node, data);

	node->key = hash;

	++hashdyn->count;
    1fca:	89 50 10             	mov    %edx,0x10(%eax)
 * Grow.
 */
tommy_inline void hashdyn_grow_step(tommy_hashdyn* hashdyn)
{
	/* grow if more than 50% full */
	if (hashdyn->count >= hashdyn->bucket_max / 2) {
    1fcd:	73 09                	jae    1fd8 <tommy_hashdyn_insert+0x58>
	node->key = hash;

	++hashdyn->count;

	hashdyn_grow_step(hashdyn);
}
    1fcf:	5b                   	pop    %ebx
    1fd0:	5e                   	pop    %esi
    1fd1:	5f                   	pop    %edi
    1fd2:	c3                   	ret    
    1fd3:	90                   	nop
    1fd4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
 */
tommy_inline void hashdyn_grow_step(tommy_hashdyn* hashdyn)
{
	/* grow if more than 50% full */
	if (hashdyn->count >= hashdyn->bucket_max / 2) {
		tommy_hashdyn_resize(hashdyn, hashdyn->bucket_bit + 1);
    1fd8:	8b 58 04             	mov    0x4(%eax),%ebx
    1fdb:	8d 53 01             	lea    0x1(%ebx),%edx
	node->key = hash;

	++hashdyn->count;

	hashdyn_grow_step(hashdyn);
}
    1fde:	5b                   	pop    %ebx
    1fdf:	5e                   	pop    %esi
    1fe0:	5f                   	pop    %edi
 */
tommy_inline void hashdyn_grow_step(tommy_hashdyn* hashdyn)
{
	/* grow if more than 50% full */
	if (hashdyn->count >= hashdyn->bucket_max / 2) {
		tommy_hashdyn_resize(hashdyn, hashdyn->bucket_bit + 1);
    1fe1:	e9 1a e0 ff ff       	jmp    0 <tommy_hashdyn_resize>
    1fe6:	8d 76 00             	lea    0x0(%esi),%esi
    1fe9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    1ff0:	89 52 04             	mov    %edx,0x4(%edx)

	/* one element "0 terminated" next list */
	node->next = 0;
    1ff3:	c7 02 00 00 00 00    	movl   $0x0,(%edx)

	*list = node;
    1ff9:	89 16                	mov    %edx,(%esi)
    1ffb:	eb ba                	jmp    1fb7 <tommy_hashdyn_insert+0x37>
    1ffd:	8d 76 00             	lea    0x0(%esi),%esi

00002000 <tommy_hashdyn_remove_existing>:

	hashdyn_grow_step(hashdyn);
}

void* tommy_hashdyn_remove_existing(tommy_hashdyn* hashdyn, tommy_hashdyn_node* node)
{
    2000:	57                   	push   %edi
    2001:	56                   	push   %esi
    2002:	53                   	push   %ebx
    2003:	8b 5c 24 14          	mov    0x14(%esp),%ebx
    2007:	8b 44 24 10          	mov    0x10(%esp),%eax
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
    200b:	8b 7b 04             	mov    0x4(%ebx),%edi
	unsigned pos = node->key & hashdyn->bucket_mask;
    200e:	8b 50 0c             	mov    0xc(%eax),%edx
    2011:	23 53 0c             	and    0xc(%ebx),%edx

	tommy_list_remove_existing(&hashdyn->bucket[pos], node);
    2014:	8b 08                	mov    (%eax),%ecx
    2016:	8d 0c 91             	lea    (%ecx,%edx,4),%ecx
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    2019:	8b 13                	mov    (%ebx),%edx
    201b:	8b 31                	mov    (%ecx),%esi
    201d:	85 d2                	test   %edx,%edx
    201f:	74 4f                	je     2070 <tommy_hashdyn_remove_existing+0x70>
		node->next->prev = node->prev;
    2021:	89 7a 04             	mov    %edi,0x4(%edx)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    2024:	39 f3                	cmp    %esi,%ebx
    2026:	74 03                	je     202b <tommy_hashdyn_remove_existing+0x2b>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    2028:	8b 4b 04             	mov    0x4(%ebx),%ecx

	--hashdyn->count;
    202b:	8b 70 10             	mov    0x10(%eax),%esi
    202e:	89 11                	mov    %edx,(%ecx)
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    2030:	8b 48 08             	mov    0x8(%eax),%ecx
{
	unsigned pos = node->key & hashdyn->bucket_mask;

	tommy_list_remove_existing(&hashdyn->bucket[pos], node);

	--hashdyn->count;
    2033:	8d 56 ff             	lea    -0x1(%esi),%edx
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    2036:	c1 e9 03             	shr    $0x3,%ecx
    2039:	39 ca                	cmp    %ecx,%edx
{
	unsigned pos = node->key & hashdyn->bucket_mask;

	tommy_list_remove_existing(&hashdyn->bucket[pos], node);

	--hashdyn->count;
    203b:	89 50 10             	mov    %edx,0x10(%eax)
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    203e:	76 10                	jbe    2050 <tommy_hashdyn_remove_existing+0x50>

	--hashdyn->count;

	hashdyn_shrink_step(hashdyn);

	return node->data;
    2040:	8b 43 08             	mov    0x8(%ebx),%eax
}
    2043:	5b                   	pop    %ebx
    2044:	5e                   	pop    %esi
    2045:	5f                   	pop    %edi
    2046:	c3                   	ret    
    2047:	89 f6                	mov    %esi,%esi
    2049:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    2050:	8b 50 04             	mov    0x4(%eax),%edx
    2053:	83 fa 04             	cmp    $0x4,%edx
    2056:	76 e8                	jbe    2040 <tommy_hashdyn_remove_existing+0x40>
		tommy_hashdyn_resize(hashdyn, hashdyn->bucket_bit - 1);
    2058:	83 ea 01             	sub    $0x1,%edx
    205b:	e8 a0 df ff ff       	call   0 <tommy_hashdyn_resize>

	--hashdyn->count;

	hashdyn_shrink_step(hashdyn);

	return node->data;
    2060:	8b 43 08             	mov    0x8(%ebx),%eax
}
    2063:	5b                   	pop    %ebx
    2064:	5e                   	pop    %esi
    2065:	5f                   	pop    %edi
    2066:	c3                   	ret    
    2067:	89 f6                	mov    %esi,%esi
    2069:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    2070:	89 7e 04             	mov    %edi,0x4(%esi)
    2073:	eb af                	jmp    2024 <tommy_hashdyn_remove_existing+0x24>
    2075:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    2079:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00002080 <tommy_hashdyn_remove>:

void* tommy_hashdyn_remove(tommy_hashdyn* hashdyn, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
    2080:	55                   	push   %ebp
    2081:	57                   	push   %edi
	unsigned pos = hash % hashdyn->bucket_max;
    2082:	31 d2                	xor    %edx,%edx

	return node->data;
}

void* tommy_hashdyn_remove(tommy_hashdyn* hashdyn, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
    2084:	56                   	push   %esi
    2085:	53                   	push   %ebx
    2086:	83 ec 1c             	sub    $0x1c,%esp
    2089:	8b 7c 24 3c          	mov    0x3c(%esp),%edi
    208d:	8b 74 24 30          	mov    0x30(%esp),%esi
    2091:	8b 6c 24 34          	mov    0x34(%esp),%ebp
	unsigned pos = hash % hashdyn->bucket_max;
    2095:	89 f8                	mov    %edi,%eax
    2097:	f7 76 08             	divl   0x8(%esi)
	tommy_hashdyn_node* i = hashdyn->bucket[pos];
    209a:	8d 04 95 00 00 00 00 	lea    0x0(,%edx,4),%eax
    20a1:	89 44 24 0c          	mov    %eax,0xc(%esp)
    20a5:	8b 06                	mov    (%esi),%eax
    20a7:	8b 1c 90             	mov    (%eax,%edx,4),%ebx

	while (i) {
    20aa:	85 db                	test   %ebx,%ebx
    20ac:	75 08                	jne    20b6 <tommy_hashdyn_remove+0x36>
    20ae:	eb 60                	jmp    2110 <tommy_hashdyn_remove+0x90>

			hashdyn_shrink_step(hashdyn);

			return i->data;
		}
		i = i->next;
    20b0:	8b 1b                	mov    (%ebx),%ebx
void* tommy_hashdyn_remove(tommy_hashdyn* hashdyn, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
	unsigned pos = hash % hashdyn->bucket_max;
	tommy_hashdyn_node* i = hashdyn->bucket[pos];

	while (i) {
    20b2:	85 db                	test   %ebx,%ebx
    20b4:	74 5a                	je     2110 <tommy_hashdyn_remove+0x90>
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
    20b6:	39 7b 0c             	cmp    %edi,0xc(%ebx)
    20b9:	75 f5                	jne    20b0 <tommy_hashdyn_remove+0x30>
    20bb:	83 ec 08             	sub    $0x8,%esp
    20be:	ff 73 08             	pushl  0x8(%ebx)
    20c1:	ff 74 24 44          	pushl  0x44(%esp)
    20c5:	ff d5                	call   *%ebp
    20c7:	83 c4 10             	add    $0x10,%esp
    20ca:	85 c0                	test   %eax,%eax
    20cc:	75 e2                	jne    20b0 <tommy_hashdyn_remove+0x30>
			tommy_list_remove_existing(&hashdyn->bucket[pos], i);
    20ce:	8b 44 24 0c          	mov    0xc(%esp),%eax
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    20d2:	8b 13                	mov    (%ebx),%edx
    20d4:	03 06                	add    (%esi),%eax
		node->next->prev = node->prev;
    20d6:	8b 7b 04             	mov    0x4(%ebx),%edi
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    20d9:	85 d2                	test   %edx,%edx
    20db:	8b 08                	mov    (%eax),%ecx
    20dd:	74 4f                	je     212e <tommy_hashdyn_remove+0xae>
		node->next->prev = node->prev;
    20df:	89 7a 04             	mov    %edi,0x4(%edx)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    20e2:	39 d9                	cmp    %ebx,%ecx
    20e4:	74 03                	je     20e9 <tommy_hashdyn_remove+0x69>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    20e6:	8b 43 04             	mov    0x4(%ebx),%eax
    20e9:	89 10                	mov    %edx,(%eax)

			--hashdyn->count;
    20eb:	8b 46 10             	mov    0x10(%esi),%eax
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    20ee:	8b 56 08             	mov    0x8(%esi),%edx
	while (i) {
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
			tommy_list_remove_existing(&hashdyn->bucket[pos], i);

			--hashdyn->count;
    20f1:	83 e8 01             	sub    $0x1,%eax
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    20f4:	c1 ea 03             	shr    $0x3,%edx
	while (i) {
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
			tommy_list_remove_existing(&hashdyn->bucket[pos], i);

			--hashdyn->count;
    20f7:	89 46 10             	mov    %eax,0x10(%esi)
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    20fa:	39 d0                	cmp    %edx,%eax
    20fc:	76 1c                	jbe    211a <tommy_hashdyn_remove+0x9a>

			--hashdyn->count;

			hashdyn_shrink_step(hashdyn);

			return i->data;
    20fe:	8b 43 08             	mov    0x8(%ebx),%eax
		}
		i = i->next;
	}

	return 0;
}
    2101:	83 c4 1c             	add    $0x1c,%esp
    2104:	5b                   	pop    %ebx
    2105:	5e                   	pop    %esi
    2106:	5f                   	pop    %edi
    2107:	5d                   	pop    %ebp
    2108:	c3                   	ret    
    2109:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    2110:	83 c4 1c             	add    $0x1c,%esp
			return i->data;
		}
		i = i->next;
	}

	return 0;
    2113:	31 c0                	xor    %eax,%eax
}
    2115:	5b                   	pop    %ebx
    2116:	5e                   	pop    %esi
    2117:	5f                   	pop    %edi
    2118:	5d                   	pop    %ebp
    2119:	c3                   	ret    
 * Shrink.
 */
tommy_inline void hashdyn_shrink_step(tommy_hashdyn* hashdyn)
{
	/* shrink if less than 12.5% full */
	if (hashdyn->count <= hashdyn->bucket_max / 8 && hashdyn->bucket_bit > TOMMY_HASHDYN_BIT) {
    211a:	8b 46 04             	mov    0x4(%esi),%eax
    211d:	83 f8 04             	cmp    $0x4,%eax
    2120:	76 dc                	jbe    20fe <tommy_hashdyn_remove+0x7e>
		tommy_hashdyn_resize(hashdyn, hashdyn->bucket_bit - 1);
    2122:	8d 50 ff             	lea    -0x1(%eax),%edx
    2125:	89 f0                	mov    %esi,%eax
    2127:	e8 d4 de ff ff       	call   0 <tommy_hashdyn_resize>
    212c:	eb d0                	jmp    20fe <tommy_hashdyn_remove+0x7e>

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    212e:	89 79 04             	mov    %edi,0x4(%ecx)
    2131:	eb af                	jmp    20e2 <tommy_hashdyn_remove+0x62>
    2133:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    2139:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00002140 <tommy_hashdyn_memory_usage>:

	return 0;
}

tommy_size_t tommy_hashdyn_memory_usage(tommy_hashdyn* hashdyn)
{
    2140:	8b 44 24 04          	mov    0x4(%esp),%eax
	return hashdyn->bucket_max * (tommy_size_t)sizeof(hashdyn->bucket[0])
		+ tommy_hashdyn_count(hashdyn) * (tommy_size_t)sizeof(tommy_hashdyn_node);
    2144:	8b 50 10             	mov    0x10(%eax),%edx
    2147:	8b 40 08             	mov    0x8(%eax),%eax
    214a:	8d 04 90             	lea    (%eax,%edx,4),%eax
	return 0;
}

tommy_size_t tommy_hashdyn_memory_usage(tommy_hashdyn* hashdyn)
{
	return hashdyn->bucket_max * (tommy_size_t)sizeof(hashdyn->bucket[0])
    214d:	c1 e0 02             	shl    $0x2,%eax
		+ tommy_hashdyn_count(hashdyn) * (tommy_size_t)sizeof(tommy_hashdyn_node);
}
    2150:	c3                   	ret    
    2151:	eb 0d                	jmp    2160 <tommy_hashlin_init>
    2153:	90                   	nop
    2154:	90                   	nop
    2155:	90                   	nop
    2156:	90                   	nop
    2157:	90                   	nop
    2158:	90                   	nop
    2159:	90                   	nop
    215a:	90                   	nop
    215b:	90                   	nop
    215c:	90                   	nop
    215d:	90                   	nop
    215e:	90                   	nop
    215f:	90                   	nop

00002160 <tommy_hashlin_init>:
#define TOMMY_HASHLIN_STATE_STABLE 0
#define TOMMY_HASHLIN_STATE_GROW 1
#define TOMMY_HASHLIN_STATE_SHRINK 2

void tommy_hashlin_init(tommy_hashlin* hashlin)
{
    2160:	57                   	push   %edi
    2161:	53                   	push   %ebx
    2162:	83 ec 10             	sub    $0x10,%esp
    2165:	8b 5c 24 1c          	mov    0x1c(%esp),%ebx
	/* fixed initial size */
	hashlin->bucket_bit = TOMMY_HASHLIN_BIT;
    2169:	c7 83 80 00 00 00 06 	movl   $0x6,0x80(%ebx)
    2170:	00 00 00 
	hashlin->bucket_max = 1 << hashlin->bucket_bit;
    2173:	c7 83 84 00 00 00 40 	movl   $0x40,0x84(%ebx)
    217a:	00 00 00 
	hashlin->bucket_mask = hashlin->bucket_max - 1;
    217d:	c7 83 88 00 00 00 3f 	movl   $0x3f,0x88(%ebx)
    2184:	00 00 00 
	hashlin->bucket[0] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->bucket_max * sizeof(tommy_hashlin_node*)));
    2187:	68 00 01 00 00       	push   $0x100
    218c:	e8 fc ff ff ff       	call   218d <tommy_hashlin_init+0x2d>
    2191:	89 c2                	mov    %eax,%edx
    2193:	89 03                	mov    %eax,(%ebx)
	memset(hashlin->bucket[0], 0, hashlin->bucket_max * sizeof(tommy_hashlin_node*));
    2195:	b9 40 00 00 00       	mov    $0x40,%ecx
    219a:	31 c0                	xor    %eax,%eax
    219c:	89 d7                	mov    %edx,%edi
	hashlin->bucket_mac = 1;
    219e:	c7 83 8c 00 00 00 01 	movl   $0x1,0x8c(%ebx)
    21a5:	00 00 00 
	/* fixed initial size */
	hashlin->bucket_bit = TOMMY_HASHLIN_BIT;
	hashlin->bucket_max = 1 << hashlin->bucket_bit;
	hashlin->bucket_mask = hashlin->bucket_max - 1;
	hashlin->bucket[0] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->bucket_max * sizeof(tommy_hashlin_node*)));
	memset(hashlin->bucket[0], 0, hashlin->bucket_max * sizeof(tommy_hashlin_node*));
    21a8:	f3 ab                	rep stos %eax,%es:(%edi)
	hashlin->bucket_mac = 1;

	/* stable state */
	hashlin->state = TOMMY_HASHLIN_STATE_STABLE;
    21aa:	c7 83 9c 00 00 00 00 	movl   $0x0,0x9c(%ebx)
    21b1:	00 00 00 

	hashlin->count = 0;
    21b4:	c7 83 a0 00 00 00 00 	movl   $0x0,0xa0(%ebx)
    21bb:	00 00 00 
}
    21be:	83 c4 14             	add    $0x14,%esp
    21c1:	5b                   	pop    %ebx
    21c2:	5f                   	pop    %edi
    21c3:	c3                   	ret    
    21c4:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    21ca:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

000021d0 <tommy_hashlin_done>:

void tommy_hashlin_done(tommy_hashlin* hashlin)
{
    21d0:	56                   	push   %esi
    21d1:	53                   	push   %ebx
	unsigned i;
	for(i=0;i<hashlin->bucket_mac;++i)
    21d2:	31 db                	xor    %ebx,%ebx

	hashlin->count = 0;
}

void tommy_hashlin_done(tommy_hashlin* hashlin)
{
    21d4:	83 ec 04             	sub    $0x4,%esp
    21d7:	8b 74 24 10          	mov    0x10(%esp),%esi
	unsigned i;
	for(i=0;i<hashlin->bucket_mac;++i)
    21db:	8b 86 8c 00 00 00    	mov    0x8c(%esi),%eax
    21e1:	85 c0                	test   %eax,%eax
    21e3:	74 1c                	je     2201 <tommy_hashlin_done+0x31>
    21e5:	8d 76 00             	lea    0x0(%esi),%esi
		tommy_free(hashlin->bucket[i]);
    21e8:	83 ec 0c             	sub    $0xc,%esp
    21eb:	ff 34 9e             	pushl  (%esi,%ebx,4)
}

void tommy_hashlin_done(tommy_hashlin* hashlin)
{
	unsigned i;
	for(i=0;i<hashlin->bucket_mac;++i)
    21ee:	83 c3 01             	add    $0x1,%ebx
		tommy_free(hashlin->bucket[i]);
    21f1:	e8 fc ff ff ff       	call   21f2 <tommy_hashlin_done+0x22>
}

void tommy_hashlin_done(tommy_hashlin* hashlin)
{
	unsigned i;
	for(i=0;i<hashlin->bucket_mac;++i)
    21f6:	83 c4 10             	add    $0x10,%esp
    21f9:	39 9e 8c 00 00 00    	cmp    %ebx,0x8c(%esi)
    21ff:	77 e7                	ja     21e8 <tommy_hashlin_done+0x18>
		tommy_free(hashlin->bucket[i]);
}
    2201:	83 c4 04             	add    $0x4,%esp
    2204:	5b                   	pop    %ebx
    2205:	5e                   	pop    %esi
    2206:	c3                   	ret    
    2207:	89 f6                	mov    %esi,%esi
    2209:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00002210 <tommy_hashlin_insert>:
		}
	}
}

void tommy_hashlin_insert(tommy_hashlin* hashlin, tommy_hashlin_node* node, void* data, tommy_hash_t hash)
{
    2210:	55                   	push   %ebp
    2211:	57                   	push   %edi
    2212:	56                   	push   %esi
    2213:	53                   	push   %ebx
    2214:	83 ec 2c             	sub    $0x2c,%esp
    2217:	8b 74 24 40          	mov    0x40(%esp),%esi
    221b:	8b 44 24 44          	mov    0x44(%esp),%eax
    221f:	8b 7c 24 4c          	mov    0x4c(%esp),%edi
tommy_inline tommy_hashlin_node** tommy_hashlin_bucket_ptr(tommy_hashlin* hashlin, tommy_hash_t hash)
{  
	unsigned pos;

	/* if we are reallocating */
	if (hashlin->state != TOMMY_HASHLIN_STATE_STABLE) {
    2223:	8b 9e 9c 00 00 00    	mov    0x9c(%esi),%ebx
    2229:	85 db                	test   %ebx,%ebx
    222b:	0f 84 6f 01 00 00    	je     23a0 <tommy_hashlin_insert+0x190>
		/* compute the old position */
		pos = hash & hashlin->low_mask;
    2231:	8b 96 94 00 00 00    	mov    0x94(%esi),%edx
    2237:	21 fa                	and    %edi,%edx

		/* if we have not reallocated this position yet */ 
		if (pos >= hashlin->split) {
    2239:	3b 96 98 00 00 00    	cmp    0x98(%esi),%edx
    223f:	0f 82 5b 01 00 00    	jb     23a0 <tommy_hashlin_insert+0x190>
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    2245:	83 fa 3f             	cmp    $0x3f,%edx
    2248:	0f 87 63 01 00 00    	ja     23b1 <tommy_hashlin_insert+0x1a1>
		return &hashlin->bucket[0][pos];
    224e:	8b 0e                	mov    (%esi),%ecx
    2250:	8d 0c 91             	lea    (%ecx,%edx,4),%ecx
    2253:	8b 11                	mov    (%ecx),%edx
 */
tommy_inline void tommy_list_insert_tail(tommy_list* list, tommy_node* node, void* data)
{
	tommy_node* head = tommy_list_head(list);

	if (head) {
    2255:	85 d2                	test   %edx,%edx
    2257:	0f 84 71 01 00 00    	je     23ce <tommy_hashlin_insert+0x1be>
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_tail_not_empty(tommy_node* head, tommy_node* node)
{
	/* insert in the "circular" prev list */
	node->prev = head->prev;
    225d:	8b 4a 04             	mov    0x4(%edx),%ecx
    2260:	89 48 04             	mov    %ecx,0x4(%eax)
	head->prev = node;
    2263:	89 42 04             	mov    %eax,0x4(%edx)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
    2266:	8b 50 04             	mov    0x4(%eax),%edx
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
    2269:	c7 00 00 00 00 00    	movl   $0x0,(%eax)
	node->prev->next = node;
    226f:	89 02                	mov    %eax,(%edx)
		tommy_list_insert_tail_not_empty(head, node);
	} else {
		tommy_list_insert_first(list, node);
	}

	node->data = data;
    2271:	8b 54 24 48          	mov    0x48(%esp),%edx

void tommy_hashlin_insert(tommy_hashlin* hashlin, tommy_hashlin_node* node, void* data, tommy_hash_t hash)
{
	tommy_list_insert_tail(tommy_hashlin_bucket_ptr(hashlin, hash), node, data);

	node->key = hash;
    2275:	89 78 0c             	mov    %edi,0xc(%eax)
 * Grow one step.
 */
tommy_inline void hashlin_grow_step(tommy_hashlin* hashlin)
{
	/* grow if more than 50% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_GROW
    2278:	83 fb 01             	cmp    $0x1,%ebx
    227b:	89 50 08             	mov    %edx,0x8(%eax)
{
	tommy_list_insert_tail(tommy_hashlin_bucket_ptr(hashlin, hash), node, data);

	node->key = hash;

	++hashlin->count;
    227e:	8b 86 a0 00 00 00    	mov    0xa0(%esi),%eax
    2284:	8d 78 01             	lea    0x1(%eax),%edi
    2287:	89 be a0 00 00 00    	mov    %edi,0xa0(%esi)
 * Grow one step.
 */
tommy_inline void hashlin_grow_step(tommy_hashlin* hashlin)
{
	/* grow if more than 50% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_GROW
    228d:	0f 84 6c 01 00 00    	je     23ff <tommy_hashlin_insert+0x1ef>
		&& hashlin->count > hashlin->bucket_max / 2
    2293:	8b ae 84 00 00 00    	mov    0x84(%esi),%ebp
    2299:	89 e8                	mov    %ebp,%eax
    229b:	d1 e8                	shr    %eax
    229d:	39 c7                	cmp    %eax,%edi
    229f:	0f 86 ef 00 00 00    	jbe    2394 <tommy_hashlin_insert+0x184>
	) {
		/* if we are stable, setup a new grow state */
		/* otherwise continue with the already setup shrink one */
		/* but in backward direction */
		if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
    22a5:	85 db                	test   %ebx,%ebx
    22a7:	0f 84 75 01 00 00    	je     2422 <tommy_hashlin_insert+0x212>
    22ad:	8b ae 90 00 00 00    	mov    0x90(%esi),%ebp
    22b3:	8b 9e 98 00 00 00    	mov    0x98(%esi),%ebx
			/* start from the beginning going forward */
			hashlin->split = 0;
		}

		/* grow state */
		hashlin->state = TOMMY_HASHLIN_STATE_GROW;
    22b9:	c7 86 9c 00 00 00 01 	movl   $0x1,0x9c(%esi)
    22c0:	00 00 00 
	}

	/* if we are growing */
	if (hashlin->state == TOMMY_HASHLIN_STATE_GROW) {
		/* compute the split target required to finish the reallocation before the next resize */
		unsigned split_target = 2 * hashlin->count;
    22c3:	8d 04 3f             	lea    (%edi,%edi,1),%eax

		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max < split_target) {
    22c6:	01 dd                	add    %ebx,%ebp
    22c8:	39 e8                	cmp    %ebp,%eax
	}

	/* if we are growing */
	if (hashlin->state == TOMMY_HASHLIN_STATE_GROW) {
		/* compute the split target required to finish the reallocation before the next resize */
		unsigned split_target = 2 * hashlin->count;
    22ca:	89 44 24 04          	mov    %eax,0x4(%esp)

		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max < split_target) {
    22ce:	0f 86 c0 00 00 00    	jbe    2394 <tommy_hashlin_insert+0x184>
    22d4:	8b 86 8c 00 00 00    	mov    0x8c(%esi),%eax
    22da:	8b be 90 00 00 00    	mov    0x90(%esi),%edi
    22e0:	89 44 24 08          	mov    %eax,0x8(%esp)
			/* reinitialize the buckets */
			*split[0] = 0;
			*split[1] = 0;

			/* compute the bit to identify the bucket */
			mask = hashlin->bucket_mask & ~hashlin->low_mask;
    22e4:	8b 86 94 00 00 00    	mov    0x94(%esi),%eax
    22ea:	f7 d0                	not    %eax
    22ec:	89 44 24 0c          	mov    %eax,0xc(%esp)
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    22f0:	83 fb 3f             	cmp    $0x3f,%ebx
    22f3:	0f 87 e7 00 00 00    	ja     23e0 <tommy_hashlin_insert+0x1d0>
		return &hashlin->bucket[0][pos];
    22f9:	8b 16                	mov    (%esi),%edx
    22fb:	c1 e3 02             	shl    $0x2,%ebx
    22fe:	01 da                	add    %ebx,%edx
			split[0] = tommy_hashlin_pos(hashlin, hashlin->split);

			/* get the high bucket */
			/* it's always in the second half, so we can index it directly */
			/* without calling tommy_hashlin_pos() */
			split[1] = &hashlin->bucket[hashlin->bucket_mac-1][hashlin->split];
    2300:	8b 44 24 08          	mov    0x8(%esp),%eax
			/* reinitialize the buckets */
			*split[0] = 0;
			*split[1] = 0;

			/* compute the bit to identify the bucket */
			mask = hashlin->bucket_mask & ~hashlin->low_mask;
    2304:	8b 6c 24 0c          	mov    0xc(%esp),%ebp
    2308:	23 ae 88 00 00 00    	and    0x88(%esi),%ebp
			tommy_hashlin_node** split[2];
			tommy_hashlin_node* j;
			unsigned mask;

			/* get the low bucket */
			split[0] = tommy_hashlin_pos(hashlin, hashlin->split);
    230e:	89 54 24 18          	mov    %edx,0x18(%esp)

			/* get the high bucket */
			/* it's always in the second half, so we can index it directly */
			/* without calling tommy_hashlin_pos() */
			split[1] = &hashlin->bucket[hashlin->bucket_mac-1][hashlin->split];
    2312:	03 5c 86 fc          	add    -0x4(%esi,%eax,4),%ebx

			/* save the low bucket */
			j = *split[0];
    2316:	8b 02                	mov    (%edx),%eax

			/* reinitialize the buckets */
			*split[0] = 0;
    2318:	c7 02 00 00 00 00    	movl   $0x0,(%edx)

			/* compute the bit to identify the bucket */
			mask = hashlin->bucket_mask & ~hashlin->low_mask;

			/* flush the bucket */
			while (j) {
    231e:	85 c0                	test   %eax,%eax
			split[0] = tommy_hashlin_pos(hashlin, hashlin->split);

			/* get the high bucket */
			/* it's always in the second half, so we can index it directly */
			/* without calling tommy_hashlin_pos() */
			split[1] = &hashlin->bucket[hashlin->bucket_mac-1][hashlin->split];
    2320:	89 5c 24 1c          	mov    %ebx,0x1c(%esp)
			/* save the low bucket */
			j = *split[0];

			/* reinitialize the buckets */
			*split[0] = 0;
			*split[1] = 0;
    2324:	c7 03 00 00 00 00    	movl   $0x0,(%ebx)

			/* compute the bit to identify the bucket */
			mask = hashlin->bucket_mask & ~hashlin->low_mask;

			/* flush the bucket */
			while (j) {
    232a:	75 1e                	jne    234a <tommy_hashlin_insert+0x13a>
    232c:	eb 42                	jmp    2370 <tommy_hashlin_insert+0x160>
    232e:	66 90                	xchg   %ax,%ax
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_tail_not_empty(tommy_node* head, tommy_node* node)
{
	/* insert in the "circular" prev list */
	node->prev = head->prev;
    2330:	8b 4a 04             	mov    0x4(%edx),%ecx
    2333:	85 db                	test   %ebx,%ebx
    2335:	89 48 04             	mov    %ecx,0x4(%eax)
	head->prev = node;
    2338:	89 42 04             	mov    %eax,0x4(%edx)

	/* insert in the "0 terminated" next list */
	node->next = 0;
	node->prev->next = node;
    233b:	8b 50 04             	mov    0x4(%eax),%edx
	/* insert in the "circular" prev list */
	node->prev = head->prev;
	head->prev = node;

	/* insert in the "0 terminated" next list */
	node->next = 0;
    233e:	c7 00 00 00 00 00    	movl   $0x0,(%eax)
	node->prev->next = node;
    2344:	89 02                	mov    %eax,(%edx)
    2346:	89 d8                	mov    %ebx,%eax
    2348:	74 26                	je     2370 <tommy_hashlin_insert+0x160>
				tommy_hashlin_node* j_next = j->next;
				unsigned index = (j->key & mask) != 0;
    234a:	31 d2                	xor    %edx,%edx
    234c:	85 68 0c             	test   %ebp,0xc(%eax)
			/* compute the bit to identify the bucket */
			mask = hashlin->bucket_mask & ~hashlin->low_mask;

			/* flush the bucket */
			while (j) {
				tommy_hashlin_node* j_next = j->next;
    234f:	8b 18                	mov    (%eax),%ebx
				unsigned index = (j->key & mask) != 0;
    2351:	0f 95 c2             	setne  %dl
				if (*split[index])
    2354:	8b 4c 94 18          	mov    0x18(%esp,%edx,4),%ecx
    2358:	8b 11                	mov    (%ecx),%edx
    235a:	85 d2                	test   %edx,%edx
    235c:	75 d2                	jne    2330 <tommy_hashlin_insert+0x120>

			/* compute the bit to identify the bucket */
			mask = hashlin->bucket_mask & ~hashlin->low_mask;

			/* flush the bucket */
			while (j) {
    235e:	85 db                	test   %ebx,%ebx
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    2360:	89 40 04             	mov    %eax,0x4(%eax)

	/* one element "0 terminated" next list */
	node->next = 0;
    2363:	c7 00 00 00 00 00    	movl   $0x0,(%eax)

	*list = node;
    2369:	89 01                	mov    %eax,(%ecx)
    236b:	89 d8                	mov    %ebx,%eax
    236d:	75 db                	jne    234a <tommy_hashlin_insert+0x13a>
    236f:	90                   	nop
					tommy_list_insert_first(split[index], j);
				j = j_next;
			}

			/* go forward */
			++hashlin->split;
    2370:	8b 86 98 00 00 00    	mov    0x98(%esi),%eax
    2376:	8d 58 01             	lea    0x1(%eax),%ebx

			/* if we have finished, change the state */
			if (hashlin->split == hashlin->low_max) {
    2379:	39 fb                	cmp    %edi,%ebx
					tommy_list_insert_first(split[index], j);
				j = j_next;
			}

			/* go forward */
			++hashlin->split;
    237b:	89 9e 98 00 00 00    	mov    %ebx,0x98(%esi)

			/* if we have finished, change the state */
			if (hashlin->split == hashlin->low_max) {
    2381:	0f 84 89 00 00 00    	je     2410 <tommy_hashlin_insert+0x200>
	if (hashlin->state == TOMMY_HASHLIN_STATE_GROW) {
		/* compute the split target required to finish the reallocation before the next resize */
		unsigned split_target = 2 * hashlin->count;

		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max < split_target) {
    2387:	8d 04 3b             	lea    (%ebx,%edi,1),%eax
    238a:	39 44 24 04          	cmp    %eax,0x4(%esp)
    238e:	0f 87 5c ff ff ff    	ja     22f0 <tommy_hashlin_insert+0xe0>
	node->key = hash;

	++hashlin->count;

	hashlin_grow_step(hashlin);
}
    2394:	83 c4 2c             	add    $0x2c,%esp
    2397:	5b                   	pop    %ebx
    2398:	5e                   	pop    %esi
    2399:	5f                   	pop    %edi
    239a:	5d                   	pop    %ebp
    239b:	c3                   	ret    
    239c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
			return tommy_hashlin_pos(hashlin, pos);
		}
	}

	/* otherwise operates normally */
	pos = hash & hashlin->bucket_mask;
    23a0:	8b 96 88 00 00 00    	mov    0x88(%esi),%edx
    23a6:	21 fa                	and    %edi,%edx
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    23a8:	83 fa 3f             	cmp    $0x3f,%edx
    23ab:	0f 86 9d fe ff ff    	jbe    224e <tommy_hashlin_insert+0x3e>
	 * Where "x ^ 31 = 31 - x", but gcc does not optimize "31 - __builtin_clz(x)" to bsr(x),
	 * but generates 31 - (bsr(x) xor 31).
	 *
	 * So we write "__builtin_clz(x) ^ 31" instead of "31 - __builtin_clz(x)".
	 */
	return __builtin_clz(value) ^ 31;
    23b1:	0f bd ca             	bsr    %edx,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    23b4:	bd 01 00 00 00       	mov    $0x1,%ebp
    23b9:	d3 e5                	shl    %cl,%ebp

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    23bb:	8b 4c 8e ec          	mov    -0x14(%esi,%ecx,4),%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    23bf:	29 ea                	sub    %ebp,%edx

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    23c1:	8d 0c 91             	lea    (%ecx,%edx,4),%ecx
    23c4:	8b 11                	mov    (%ecx),%edx
 */
tommy_inline void tommy_list_insert_tail(tommy_list* list, tommy_node* node, void* data)
{
	tommy_node* head = tommy_list_head(list);

	if (head) {
    23c6:	85 d2                	test   %edx,%edx
    23c8:	0f 85 8f fe ff ff    	jne    225d <tommy_hashlin_insert+0x4d>
 * \param node The node to insert.
 */
tommy_inline void tommy_list_insert_first(tommy_list* list, tommy_node* node)
{
	/* one element "circular" prev list */   
	node->prev = node;
    23ce:	89 40 04             	mov    %eax,0x4(%eax)

	/* one element "0 terminated" next list */
	node->next = 0;
    23d1:	c7 00 00 00 00 00    	movl   $0x0,(%eax)

	*list = node;
    23d7:	89 01                	mov    %eax,(%ecx)
    23d9:	e9 93 fe ff ff       	jmp    2271 <tommy_hashlin_insert+0x61>
    23de:	66 90                	xchg   %ax,%ax
    23e0:	0f bd cb             	bsr    %ebx,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    23e3:	b8 01 00 00 00       	mov    $0x1,%eax
    23e8:	89 da                	mov    %ebx,%edx
    23ea:	d3 e0                	shl    %cl,%eax
    23ec:	c1 e3 02             	shl    $0x2,%ebx
    23ef:	29 c2                	sub    %eax,%edx
    23f1:	89 d0                	mov    %edx,%eax

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    23f3:	8b 54 8e ec          	mov    -0x14(%esi,%ecx,4),%edx
    23f7:	8d 14 82             	lea    (%edx,%eax,4),%edx
    23fa:	e9 01 ff ff ff       	jmp    2300 <tommy_hashlin_insert+0xf0>
    23ff:	8b ae 90 00 00 00    	mov    0x90(%esi),%ebp
    2405:	8b 9e 98 00 00 00    	mov    0x98(%esi),%ebx
    240b:	e9 b3 fe ff ff       	jmp    22c3 <tommy_hashlin_insert+0xb3>
			/* go forward */
			++hashlin->split;

			/* if we have finished, change the state */
			if (hashlin->split == hashlin->low_max) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;
    2410:	c7 86 9c 00 00 00 00 	movl   $0x0,0x9c(%esi)
    2417:	00 00 00 
	node->key = hash;

	++hashlin->count;

	hashlin_grow_step(hashlin);
}
    241a:	83 c4 2c             	add    $0x2c,%esp
    241d:	5b                   	pop    %ebx
    241e:	5e                   	pop    %esi
    241f:	5f                   	pop    %edi
    2420:	5d                   	pop    %ebp
    2421:	c3                   	ret    
		/* otherwise continue with the already setup shrink one */
		/* but in backward direction */
		if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
			/* set the lower size */
			hashlin->low_max = hashlin->bucket_max;
			hashlin->low_mask = hashlin->bucket_mask;
    2422:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax

			/* grow the hash size and allocate */
			++hashlin->bucket_bit;
			hashlin->bucket_max = 1 << hashlin->bucket_bit;
			hashlin->bucket_mask = hashlin->bucket_max - 1;
			hashlin->bucket[hashlin->bucket_mac] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->low_max * sizeof(tommy_hashlin_node*)));
    2428:	8b 96 8c 00 00 00    	mov    0x8c(%esi),%edx
    242e:	83 ec 0c             	sub    $0xc,%esp
		/* if we are stable, setup a new grow state */
		/* otherwise continue with the already setup shrink one */
		/* but in backward direction */
		if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
			/* set the lower size */
			hashlin->low_max = hashlin->bucket_max;
    2431:	89 ae 90 00 00 00    	mov    %ebp,0x90(%esi)
			hashlin->low_mask = hashlin->bucket_mask;
    2437:	89 86 94 00 00 00    	mov    %eax,0x94(%esi)

			/* grow the hash size and allocate */
			++hashlin->bucket_bit;
    243d:	8b 86 80 00 00 00    	mov    0x80(%esi),%eax
			hashlin->bucket_max = 1 << hashlin->bucket_bit;
			hashlin->bucket_mask = hashlin->bucket_max - 1;
			hashlin->bucket[hashlin->bucket_mac] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->low_max * sizeof(tommy_hashlin_node*)));
    2443:	89 54 24 10          	mov    %edx,0x10(%esp)
			/* set the lower size */
			hashlin->low_max = hashlin->bucket_max;
			hashlin->low_mask = hashlin->bucket_mask;

			/* grow the hash size and allocate */
			++hashlin->bucket_bit;
    2447:	8d 48 01             	lea    0x1(%eax),%ecx
			hashlin->bucket_max = 1 << hashlin->bucket_bit;
    244a:	b8 01 00 00 00       	mov    $0x1,%eax
    244f:	d3 e0                	shl    %cl,%eax
			/* set the lower size */
			hashlin->low_max = hashlin->bucket_max;
			hashlin->low_mask = hashlin->bucket_mask;

			/* grow the hash size and allocate */
			++hashlin->bucket_bit;
    2451:	89 8e 80 00 00 00    	mov    %ecx,0x80(%esi)
			hashlin->bucket_max = 1 << hashlin->bucket_bit;
    2457:	89 86 84 00 00 00    	mov    %eax,0x84(%esi)
			hashlin->bucket_mask = hashlin->bucket_max - 1;
    245d:	83 e8 01             	sub    $0x1,%eax
    2460:	89 86 88 00 00 00    	mov    %eax,0x88(%esi)
			hashlin->bucket[hashlin->bucket_mac] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->low_max * sizeof(tommy_hashlin_node*)));
    2466:	8d 04 ad 00 00 00 00 	lea    0x0(,%ebp,4),%eax
    246d:	50                   	push   %eax
    246e:	e8 fc ff ff ff       	call   246f <tommy_hashlin_insert+0x25f>
    2473:	8b 54 24 14          	mov    0x14(%esp),%edx
			++hashlin->bucket_mac;

			/* start from the beginning going forward */
			hashlin->split = 0;
    2477:	83 c4 10             	add    $0x10,%esp

			/* grow the hash size and allocate */
			++hashlin->bucket_bit;
			hashlin->bucket_max = 1 << hashlin->bucket_bit;
			hashlin->bucket_mask = hashlin->bucket_max - 1;
			hashlin->bucket[hashlin->bucket_mac] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->low_max * sizeof(tommy_hashlin_node*)));
    247a:	89 04 96             	mov    %eax,(%esi,%edx,4)
			++hashlin->bucket_mac;
    247d:	83 c2 01             	add    $0x1,%edx

			/* start from the beginning going forward */
			hashlin->split = 0;
    2480:	c7 86 98 00 00 00 00 	movl   $0x0,0x98(%esi)
    2487:	00 00 00 
			/* grow the hash size and allocate */
			++hashlin->bucket_bit;
			hashlin->bucket_max = 1 << hashlin->bucket_bit;
			hashlin->bucket_mask = hashlin->bucket_max - 1;
			hashlin->bucket[hashlin->bucket_mac] = tommy_cast(tommy_hashlin_node**, tommy_malloc(hashlin->low_max * sizeof(tommy_hashlin_node*)));
			++hashlin->bucket_mac;
    248a:	89 96 8c 00 00 00    	mov    %edx,0x8c(%esi)
    2490:	e9 24 fe ff ff       	jmp    22b9 <tommy_hashlin_insert+0xa9>
    2495:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    2499:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000024a0 <tommy_hashlin_remove_existing>:

	hashlin_grow_step(hashlin);
}

void* tommy_hashlin_remove_existing(tommy_hashlin* hashlin, tommy_hashlin_node* node)
{
    24a0:	55                   	push   %ebp
    24a1:	57                   	push   %edi
    24a2:	56                   	push   %esi
    24a3:	53                   	push   %ebx
    24a4:	83 ec 1c             	sub    $0x1c,%esp
    24a7:	8b 54 24 30          	mov    0x30(%esp),%edx
	tommy_list_remove_existing(tommy_hashlin_bucket_ptr(hashlin, node->key), node);
    24ab:	8b 44 24 34          	mov    0x34(%esp),%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_bucket_ptr(tommy_hashlin* hashlin, tommy_hash_t hash)
{  
	unsigned pos;

	/* if we are reallocating */
	if (hashlin->state != TOMMY_HASHLIN_STATE_STABLE) {
    24af:	8b 9a 9c 00 00 00    	mov    0x9c(%edx),%ebx
	hashlin_grow_step(hashlin);
}

void* tommy_hashlin_remove_existing(tommy_hashlin* hashlin, tommy_hashlin_node* node)
{
	tommy_list_remove_existing(tommy_hashlin_bucket_ptr(hashlin, node->key), node);
    24b5:	8b 40 0c             	mov    0xc(%eax),%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_bucket_ptr(tommy_hashlin* hashlin, tommy_hash_t hash)
{  
	unsigned pos;

	/* if we are reallocating */
	if (hashlin->state != TOMMY_HASHLIN_STATE_STABLE) {
    24b8:	85 db                	test   %ebx,%ebx
    24ba:	74 24                	je     24e0 <tommy_hashlin_remove_existing+0x40>
		/* compute the old position */
		pos = hash & hashlin->low_mask;
    24bc:	8b b2 94 00 00 00    	mov    0x94(%edx),%esi
    24c2:	21 c6                	and    %eax,%esi

		/* if we have not reallocated this position yet */ 
		if (pos >= hashlin->split) {
    24c4:	3b b2 98 00 00 00    	cmp    0x98(%edx),%esi
    24ca:	72 14                	jb     24e0 <tommy_hashlin_remove_existing+0x40>
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    24cc:	83 fe 3f             	cmp    $0x3f,%esi
    24cf:	0f 87 ab 01 00 00    	ja     2680 <tommy_hashlin_remove_existing+0x1e0>
		return &hashlin->bucket[0][pos];
    24d5:	8b 02                	mov    (%edx),%eax
    24d7:	8d 34 b0             	lea    (%eax,%esi,4),%esi
    24da:	eb 26                	jmp    2502 <tommy_hashlin_remove_existing+0x62>
    24dc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
			return tommy_hashlin_pos(hashlin, pos);
		}
	}

	/* otherwise operates normally */
	pos = hash & hashlin->bucket_mask;
    24e0:	23 82 88 00 00 00    	and    0x88(%edx),%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    24e6:	83 f8 3f             	cmp    $0x3f,%eax
    24e9:	0f 86 29 01 00 00    	jbe    2618 <tommy_hashlin_remove_existing+0x178>
    24ef:	0f bd c8             	bsr    %eax,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    24f2:	be 01 00 00 00       	mov    $0x1,%esi
    24f7:	d3 e6                	shl    %cl,%esi

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    24f9:	8b 4c 8a ec          	mov    -0x14(%edx,%ecx,4),%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    24fd:	29 f0                	sub    %esi,%eax

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    24ff:	8d 34 81             	lea    (%ecx,%eax,4),%esi
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    2502:	8b 44 24 34          	mov    0x34(%esp),%eax
		node->next->prev = node->prev;
    2506:	8b 7c 24 34          	mov    0x34(%esp),%edi
    250a:	8b 0e                	mov    (%esi),%ecx
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    250c:	8b 00                	mov    (%eax),%eax
		node->next->prev = node->prev;
    250e:	8b 7f 04             	mov    0x4(%edi),%edi
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    2511:	85 c0                	test   %eax,%eax
    2513:	0f 84 af 01 00 00    	je     26c8 <tommy_hashlin_remove_existing+0x228>
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    2519:	39 4c 24 34          	cmp    %ecx,0x34(%esp)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
    251d:	89 78 04             	mov    %edi,0x4(%eax)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    2520:	0f 84 b2 01 00 00    	je     26d8 <tommy_hashlin_remove_existing+0x238>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    2526:	8b 74 24 34          	mov    0x34(%esp),%esi
    252a:	8b 4e 04             	mov    0x4(%esi),%ecx
    252d:	89 01                	mov    %eax,(%ecx)

void* tommy_hashlin_remove_existing(tommy_hashlin* hashlin, tommy_hashlin_node* node)
{
	tommy_list_remove_existing(tommy_hashlin_bucket_ptr(hashlin, node->key), node);

	--hashlin->count;
    252f:	8b 82 a0 00 00 00    	mov    0xa0(%edx),%eax
 * Shrink one step.
 */
tommy_inline void hashlin_shrink_step(tommy_hashlin* hashlin)
{
	/* shrink if less than 12.5% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_SHRINK
    2535:	83 fb 02             	cmp    $0x2,%ebx

void* tommy_hashlin_remove_existing(tommy_hashlin* hashlin, tommy_hashlin_node* node)
{
	tommy_list_remove_existing(tommy_hashlin_bucket_ptr(hashlin, node->key), node);

	--hashlin->count;
    2538:	8d 48 ff             	lea    -0x1(%eax),%ecx
    253b:	89 8a a0 00 00 00    	mov    %ecx,0xa0(%edx)
 * Shrink one step.
 */
tommy_inline void hashlin_shrink_step(tommy_hashlin* hashlin)
{
	/* shrink if less than 12.5% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_SHRINK
    2541:	0f 84 69 01 00 00    	je     26b0 <tommy_hashlin_remove_existing+0x210>
		&& hashlin->count < hashlin->bucket_max / 8
    2547:	8b 82 84 00 00 00    	mov    0x84(%edx),%eax
    254d:	89 c6                	mov    %eax,%esi
    254f:	c1 ee 03             	shr    $0x3,%esi
    2552:	39 f1                	cmp    %esi,%ecx
    2554:	0f 83 11 01 00 00    	jae    266b <tommy_hashlin_remove_existing+0x1cb>
	) {
		/* avoid to shrink the first bucket */
		if (hashlin->bucket_bit > TOMMY_HASHLIN_BIT) {
    255a:	83 ba 80 00 00 00 06 	cmpl   $0x6,0x80(%edx)
    2561:	0f 86 04 01 00 00    	jbe    266b <tommy_hashlin_remove_existing+0x1cb>
			/* if we are stable, setup a new shrink state */
			/* otherwise continue with the already setup grow one */
			/* but in backward direction */
			if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
    2567:	85 db                	test   %ebx,%ebx
    2569:	0f 84 71 01 00 00    	je     26e0 <tommy_hashlin_remove_existing+0x240>
    256f:	8b 82 90 00 00 00    	mov    0x90(%edx),%eax
    2575:	89 44 24 0c          	mov    %eax,0xc(%esp)
    2579:	8b 82 98 00 00 00    	mov    0x98(%edx),%eax
				/* start from the half going backward */
				hashlin->split = hashlin->low_max;
			}

			/* start reallocation */
			hashlin->state = TOMMY_HASHLIN_STATE_SHRINK;
    257f:	c7 82 9c 00 00 00 02 	movl   $0x2,0x9c(%edx)
    2586:	00 00 00 
	}

	/* if we are shrinking */
	if (hashlin->state == TOMMY_HASHLIN_STATE_SHRINK) {
		/* compute the split target required to finish the reallocation before the next resize */
		unsigned split_target = 8 * hashlin->count;
    2589:	8d 34 cd 00 00 00 00 	lea    0x0(,%ecx,8),%esi
    2590:	89 74 24 08          	mov    %esi,0x8(%esp)
    2594:	8d 34 85 fc ff ff ff 	lea    -0x4(,%eax,4),%esi
    259b:	eb 42                	jmp    25df <tommy_hashlin_remove_existing+0x13f>
    259d:	8d 76 00             	lea    0x0(%esi),%esi
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
		return &hashlin->bucket[0][pos];
    25a0:	8b 2a                	mov    (%edx),%ebp
    25a2:	01 f5                	add    %esi,%ebp
			split[0] = tommy_hashlin_pos(hashlin, hashlin->split);

			/* get the high bucket */
			/* it's always in the second half, so we can index it directly */
			/* without calling tommy_hashlin_pos() */
			split[1] = &hashlin->bucket[hashlin->bucket_mac-1][hashlin->split];
    25a4:	8b ba 8c 00 00 00    	mov    0x8c(%edx),%edi
    25aa:	8d 5f ff             	lea    -0x1(%edi),%ebx
    25ad:	8b 0c 9a             	mov    (%edx,%ebx,4),%ecx
    25b0:	8b 0c 31             	mov    (%ecx,%esi,1),%ecx
{
	tommy_node* first_head;
	tommy_node* first_tail;
	tommy_node* second_head;

	if (tommy_list_empty(second)) {
    25b3:	85 c9                	test   %ecx,%ecx
    25b5:	74 21                	je     25d8 <tommy_hashlin_remove_existing+0x138>
    25b7:	8b 7d 00             	mov    0x0(%ebp),%edi
		return;
	}

	if (tommy_list_empty(first)) {
    25ba:	85 ff                	test   %edi,%edi
    25bc:	0f 84 de 00 00 00    	je     26a0 <tommy_hashlin_remove_existing+0x200>
	tommy_node* head = tommy_list_head(list);

	if (!head)
		return 0;

	return head->prev;
    25c2:	8b 6f 04             	mov    0x4(%edi),%ebp
    25c5:	89 6c 24 04          	mov    %ebp,0x4(%esp)
	first_head = tommy_list_head(first);
	second_head = tommy_list_head(second);
	first_tail = tommy_list_tail(first);

	/* set the "circular" prev list */
	first_head->prev = second_head->prev;
    25c9:	8b 69 04             	mov    0x4(%ecx),%ebp
    25cc:	89 6f 04             	mov    %ebp,0x4(%edi)
	second_head->prev = first_tail;
    25cf:	8b 7c 24 04          	mov    0x4(%esp),%edi
    25d3:	89 79 04             	mov    %edi,0x4(%ecx)

	/* set the "0 terminated" next list */
	first_tail->next = second_head;
    25d6:	89 0f                	mov    %ecx,(%edi)
    25d8:	83 ee 04             	sub    $0x4,%esi

			/* concat the high bucket into the low one */
			tommy_list_concat(split[0], split[1]);

			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
    25db:	85 c0                	test   %eax,%eax
    25dd:	74 49                	je     2628 <tommy_hashlin_remove_existing+0x188>
    25df:	8b 7c 24 0c          	mov    0xc(%esp),%edi
    25e3:	8d 0c 38             	lea    (%eax,%edi,1),%ecx
	if (hashlin->state == TOMMY_HASHLIN_STATE_SHRINK) {
		/* compute the split target required to finish the reallocation before the next resize */
		unsigned split_target = 8 * hashlin->count;

		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max > split_target) {
    25e6:	39 4c 24 08          	cmp    %ecx,0x8(%esp)
    25ea:	73 7f                	jae    266b <tommy_hashlin_remove_existing+0x1cb>
			tommy_hashlin_node** split[2];

			/* go backward position */
			--hashlin->split;
    25ec:	83 e8 01             	sub    $0x1,%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    25ef:	83 f8 3f             	cmp    $0x3f,%eax
		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max > split_target) {
			tommy_hashlin_node** split[2];

			/* go backward position */
			--hashlin->split;
    25f2:	89 82 98 00 00 00    	mov    %eax,0x98(%edx)
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    25f8:	76 a6                	jbe    25a0 <tommy_hashlin_remove_existing+0x100>
    25fa:	0f bd c8             	bsr    %eax,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    25fd:	bb 01 00 00 00       	mov    $0x1,%ebx
    2602:	89 c7                	mov    %eax,%edi
    2604:	d3 e3                	shl    %cl,%ebx

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    2606:	8b 4c 8a ec          	mov    -0x14(%edx,%ecx,4),%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    260a:	29 df                	sub    %ebx,%edi

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    260c:	8d 2c b9             	lea    (%ecx,%edi,4),%ebp
    260f:	eb 93                	jmp    25a4 <tommy_hashlin_remove_existing+0x104>
    2611:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
		return &hashlin->bucket[0][pos];
    2618:	8b 0a                	mov    (%edx),%ecx
    261a:	8d 34 81             	lea    (%ecx,%eax,4),%esi
    261d:	e9 e0 fe ff ff       	jmp    2502 <tommy_hashlin_remove_existing+0x62>
    2622:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;

				/* shrink the hash size */
				--hashlin->bucket_bit;
    2628:	8b 82 80 00 00 00    	mov    0x80(%edx),%eax
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
				hashlin->bucket_mask = hashlin->bucket_max - 1;

				/* free the last segment */
				--hashlin->bucket_mac;
				tommy_free(hashlin->bucket[hashlin->bucket_mac]);
    262e:	83 ec 0c             	sub    $0xc,%esp
			/* concat the high bucket into the low one */
			tommy_list_concat(split[0], split[1]);

			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;
    2631:	c7 82 9c 00 00 00 00 	movl   $0x0,0x9c(%edx)
    2638:	00 00 00 
				--hashlin->bucket_bit;
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
				hashlin->bucket_mask = hashlin->bucket_max - 1;

				/* free the last segment */
				--hashlin->bucket_mac;
    263b:	89 9a 8c 00 00 00    	mov    %ebx,0x8c(%edx)
			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;

				/* shrink the hash size */
				--hashlin->bucket_bit;
    2641:	8d 48 ff             	lea    -0x1(%eax),%ecx
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
    2644:	b8 01 00 00 00       	mov    $0x1,%eax
    2649:	d3 e0                	shl    %cl,%eax
			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;

				/* shrink the hash size */
				--hashlin->bucket_bit;
    264b:	89 8a 80 00 00 00    	mov    %ecx,0x80(%edx)
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
    2651:	89 82 84 00 00 00    	mov    %eax,0x84(%edx)
				hashlin->bucket_mask = hashlin->bucket_max - 1;
    2657:	83 e8 01             	sub    $0x1,%eax
    265a:	89 82 88 00 00 00    	mov    %eax,0x88(%edx)

				/* free the last segment */
				--hashlin->bucket_mac;
				tommy_free(hashlin->bucket[hashlin->bucket_mac]);
    2660:	ff 34 9a             	pushl  (%edx,%ebx,4)
    2663:	e8 fc ff ff ff       	call   2664 <tommy_hashlin_remove_existing+0x1c4>
    2668:	83 c4 10             	add    $0x10,%esp

	--hashlin->count;

	hashlin_shrink_step(hashlin);

	return node->data;
    266b:	8b 44 24 34          	mov    0x34(%esp),%eax
    266f:	8b 40 08             	mov    0x8(%eax),%eax
}
    2672:	83 c4 1c             	add    $0x1c,%esp
    2675:	5b                   	pop    %ebx
    2676:	5e                   	pop    %esi
    2677:	5f                   	pop    %edi
    2678:	5d                   	pop    %ebp
    2679:	c3                   	ret    
    267a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    2680:	0f bd ce             	bsr    %esi,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    2683:	b8 01 00 00 00       	mov    $0x1,%eax
    2688:	d3 e0                	shl    %cl,%eax
    268a:	29 c6                	sub    %eax,%esi

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    268c:	8b 44 8a ec          	mov    -0x14(%edx,%ecx,4),%eax
    2690:	8d 34 b0             	lea    (%eax,%esi,4),%esi
    2693:	e9 6a fe ff ff       	jmp    2502 <tommy_hashlin_remove_existing+0x62>
    2698:	90                   	nop
    2699:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
	if (tommy_list_empty(second)) {
		return;
	}

	if (tommy_list_empty(first)) {
		*first = *second;
    26a0:	89 4d 00             	mov    %ecx,0x0(%ebp)
    26a3:	e9 30 ff ff ff       	jmp    25d8 <tommy_hashlin_remove_existing+0x138>
    26a8:	90                   	nop
    26a9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    26b0:	8b 82 90 00 00 00    	mov    0x90(%edx),%eax
    26b6:	89 44 24 0c          	mov    %eax,0xc(%esp)
    26ba:	8b 82 98 00 00 00    	mov    0x98(%edx),%eax
    26c0:	e9 c4 fe ff ff       	jmp    2589 <tommy_hashlin_remove_existing+0xe9>
    26c5:	8d 76 00             	lea    0x0(%esi),%esi
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    26c8:	39 4c 24 34          	cmp    %ecx,0x34(%esp)

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    26cc:	89 79 04             	mov    %edi,0x4(%ecx)
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    26cf:	0f 85 51 fe ff ff    	jne    2526 <tommy_hashlin_remove_existing+0x86>
    26d5:	8d 76 00             	lea    0x0(%esi),%esi
		*list = node->next; /* the new head, in case 0 */
    26d8:	89 06                	mov    %eax,(%esi)
    26da:	e9 50 fe ff ff       	jmp    252f <tommy_hashlin_remove_existing+0x8f>
    26df:	90                   	nop
			/* if we are stable, setup a new shrink state */
			/* otherwise continue with the already setup grow one */
			/* but in backward direction */
			if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
				/* set the lower size */
				hashlin->low_max = hashlin->bucket_max / 2;
    26e0:	d1 e8                	shr    %eax
    26e2:	89 c6                	mov    %eax,%esi
    26e4:	89 44 24 0c          	mov    %eax,0xc(%esp)
    26e8:	89 82 90 00 00 00    	mov    %eax,0x90(%edx)
				hashlin->low_mask = hashlin->bucket_mask / 2;
    26ee:	8b 82 88 00 00 00    	mov    0x88(%edx),%eax

				/* start from the half going backward */
				hashlin->split = hashlin->low_max;
    26f4:	89 b2 98 00 00 00    	mov    %esi,0x98(%edx)
			/* otherwise continue with the already setup grow one */
			/* but in backward direction */
			if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
				/* set the lower size */
				hashlin->low_max = hashlin->bucket_max / 2;
				hashlin->low_mask = hashlin->bucket_mask / 2;
    26fa:	d1 e8                	shr    %eax
    26fc:	89 82 94 00 00 00    	mov    %eax,0x94(%edx)

				/* start from the half going backward */
				hashlin->split = hashlin->low_max;
    2702:	89 f0                	mov    %esi,%eax
    2704:	e9 76 fe ff ff       	jmp    257f <tommy_hashlin_remove_existing+0xdf>
    2709:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00002710 <tommy_hashlin_bucket>:

	return node->data;
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
    2710:	53                   	push   %ebx
    2711:	8b 54 24 08          	mov    0x8(%esp),%edx
    2715:	8b 44 24 0c          	mov    0xc(%esp),%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_bucket_ptr(tommy_hashlin* hashlin, tommy_hash_t hash)
{  
	unsigned pos;

	/* if we are reallocating */
	if (hashlin->state != TOMMY_HASHLIN_STATE_STABLE) {
    2719:	8b 8a 9c 00 00 00    	mov    0x9c(%edx),%ecx
    271f:	85 c9                	test   %ecx,%ecx
    2721:	74 25                	je     2748 <tommy_hashlin_bucket+0x38>
		/* compute the old position */
		pos = hash & hashlin->low_mask;
    2723:	8b 9a 94 00 00 00    	mov    0x94(%edx),%ebx
    2729:	21 c3                	and    %eax,%ebx

		/* if we have not reallocated this position yet */ 
		if (pos >= hashlin->split) {
    272b:	3b 9a 98 00 00 00    	cmp    0x98(%edx),%ebx
    2731:	72 15                	jb     2748 <tommy_hashlin_bucket+0x38>
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    2733:	83 fb 3f             	cmp    $0x3f,%ebx
    2736:	77 48                	ja     2780 <tommy_hashlin_bucket+0x70>
		return &hashlin->bucket[0][pos];
    2738:	8b 02                	mov    (%edx),%eax
    273a:	8d 04 98             	lea    (%eax,%ebx,4),%eax
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
}
    273d:	5b                   	pop    %ebx
	return node->data;
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
    273e:	8b 00                	mov    (%eax),%eax
}
    2740:	c3                   	ret    
    2741:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
			return tommy_hashlin_pos(hashlin, pos);
		}
	}

	/* otherwise operates normally */
	pos = hash & hashlin->bucket_mask;
    2748:	23 82 88 00 00 00    	and    0x88(%edx),%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    274e:	83 f8 3f             	cmp    $0x3f,%eax
    2751:	76 1d                	jbe    2770 <tommy_hashlin_bucket+0x60>
    2753:	0f bd c8             	bsr    %eax,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    2756:	bb 01 00 00 00       	mov    $0x1,%ebx

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    275b:	8b 54 8a ec          	mov    -0x14(%edx,%ecx,4),%edx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    275f:	d3 e3                	shl    %cl,%ebx
    2761:	29 d8                	sub    %ebx,%eax
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
}
    2763:	5b                   	pop    %ebx
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    2764:	8d 04 82             	lea    (%edx,%eax,4),%eax
	return node->data;
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
    2767:	8b 00                	mov    (%eax),%eax
}
    2769:	c3                   	ret    
    276a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
		return &hashlin->bucket[0][pos];
    2770:	8b 12                	mov    (%edx),%edx
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
}
    2772:	5b                   	pop    %ebx
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
		return &hashlin->bucket[0][pos];
    2773:	8d 04 82             	lea    (%edx,%eax,4),%eax
	return node->data;
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
    2776:	8b 00                	mov    (%eax),%eax
}
    2778:	c3                   	ret    
    2779:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
    2780:	0f bd cb             	bsr    %ebx,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    2783:	b8 01 00 00 00       	mov    $0x1,%eax
    2788:	d3 e0                	shl    %cl,%eax
    278a:	29 c3                	sub    %eax,%ebx

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    278c:	8b 44 8a ec          	mov    -0x14(%edx,%ecx,4),%eax
    2790:	8d 04 98             	lea    (%eax,%ebx,4),%eax
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
}
    2793:	5b                   	pop    %ebx
	return node->data;
}

tommy_hashlin_node* tommy_hashlin_bucket(tommy_hashlin* hashlin, tommy_hash_t hash)
{
	return *tommy_hashlin_bucket_ptr(hashlin, hash);
    2794:	8b 00                	mov    (%eax),%eax
}
    2796:	c3                   	ret    
    2797:	89 f6                	mov    %esi,%esi
    2799:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000027a0 <tommy_hashlin_remove>:

void* tommy_hashlin_remove(tommy_hashlin* hashlin, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
    27a0:	55                   	push   %ebp
    27a1:	57                   	push   %edi
    27a2:	56                   	push   %esi
    27a3:	53                   	push   %ebx
    27a4:	83 ec 1c             	sub    $0x1c,%esp
    27a7:	8b 74 24 30          	mov    0x30(%esp),%esi
    27ab:	8b 7c 24 3c          	mov    0x3c(%esp),%edi
tommy_inline tommy_hashlin_node** tommy_hashlin_bucket_ptr(tommy_hashlin* hashlin, tommy_hash_t hash)
{  
	unsigned pos;

	/* if we are reallocating */
	if (hashlin->state != TOMMY_HASHLIN_STATE_STABLE) {
    27af:	8b 86 9c 00 00 00    	mov    0x9c(%esi),%eax
    27b5:	85 c0                	test   %eax,%eax
    27b7:	0f 84 c3 01 00 00    	je     2980 <tommy_hashlin_remove+0x1e0>
		/* compute the old position */
		pos = hash & hashlin->low_mask;
    27bd:	8b 86 94 00 00 00    	mov    0x94(%esi),%eax
    27c3:	21 f8                	and    %edi,%eax

		/* if we have not reallocated this position yet */ 
		if (pos >= hashlin->split) {
    27c5:	3b 86 98 00 00 00    	cmp    0x98(%esi),%eax
    27cb:	0f 82 af 01 00 00    	jb     2980 <tommy_hashlin_remove+0x1e0>
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    27d1:	83 f8 3f             	cmp    $0x3f,%eax
    27d4:	0f 87 b7 01 00 00    	ja     2991 <tommy_hashlin_remove+0x1f1>
		return &hashlin->bucket[0][pos];
    27da:	8b 0e                	mov    (%esi),%ecx
    27dc:	8d 2c 81             	lea    (%ecx,%eax,4),%ebp
}

void* tommy_hashlin_remove(tommy_hashlin* hashlin, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
	tommy_hashlin_node** let_ptr = tommy_hashlin_bucket_ptr(hashlin, hash);
	tommy_hashlin_node* i = *let_ptr;
    27df:	8b 5d 00             	mov    0x0(%ebp),%ebx

	while (i) {
    27e2:	85 db                	test   %ebx,%ebx
    27e4:	0f 84 c6 01 00 00    	je     29b0 <tommy_hashlin_remove+0x210>
    27ea:	89 2c 24             	mov    %ebp,(%esp)
    27ed:	89 74 24 30          	mov    %esi,0x30(%esp)
    27f1:	89 fd                	mov    %edi,%ebp
    27f3:	89 de                	mov    %ebx,%esi
    27f5:	8b 7c 24 34          	mov    0x34(%esp),%edi
    27f9:	8b 5c 24 38          	mov    0x38(%esp),%ebx
    27fd:	eb 0b                	jmp    280a <tommy_hashlin_remove+0x6a>
    27ff:	90                   	nop

			hashlin_shrink_step(hashlin);

			return i->data;
		}
		i = i->next;
    2800:	8b 36                	mov    (%esi),%esi
void* tommy_hashlin_remove(tommy_hashlin* hashlin, tommy_search_func* cmp, const void* cmp_arg, tommy_hash_t hash)
{
	tommy_hashlin_node** let_ptr = tommy_hashlin_bucket_ptr(hashlin, hash);
	tommy_hashlin_node* i = *let_ptr;

	while (i) {
    2802:	85 f6                	test   %esi,%esi
    2804:	0f 84 a6 01 00 00    	je     29b0 <tommy_hashlin_remove+0x210>
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
    280a:	39 6e 0c             	cmp    %ebp,0xc(%esi)
    280d:	75 f1                	jne    2800 <tommy_hashlin_remove+0x60>
    280f:	83 ec 08             	sub    $0x8,%esp
    2812:	ff 76 08             	pushl  0x8(%esi)
    2815:	53                   	push   %ebx
    2816:	ff d7                	call   *%edi
    2818:	83 c4 10             	add    $0x10,%esp
    281b:	85 c0                	test   %eax,%eax
    281d:	75 e1                	jne    2800 <tommy_hashlin_remove+0x60>
    281f:	89 f3                	mov    %esi,%ebx
    2821:	8b 2c 24             	mov    (%esp),%ebp
    2824:	8b 74 24 30          	mov    0x30(%esp),%esi
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    2828:	8b 03                	mov    (%ebx),%eax
		node->next->prev = node->prev;
    282a:	8b 4b 04             	mov    0x4(%ebx),%ecx
    282d:	8b 55 00             	mov    0x0(%ebp),%edx
tommy_inline void* tommy_list_remove_existing(tommy_list* list, tommy_node* node)
{
	tommy_node* head = tommy_list_head(list);

	/* remove from the "circular" prev list */
	if (node->next) { 
    2830:	85 c0                	test   %eax,%eax
    2832:	0f 84 c8 01 00 00    	je     2a00 <tommy_hashlin_remove+0x260>
		node->next->prev = node->prev;
    2838:	89 48 04             	mov    %ecx,0x4(%eax)
	} else {
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
    283b:	39 da                	cmp    %ebx,%edx
    283d:	0f 84 b5 01 00 00    	je     29f8 <tommy_hashlin_remove+0x258>
		*list = node->next; /* the new head, in case 0 */
	} else {
		node->prev->next = node->next;
    2843:	8b 53 04             	mov    0x4(%ebx),%edx
    2846:	89 02                	mov    %eax,(%edx)
			tommy_list_remove_existing(let_ptr, i);

			--hashlin->count;
    2848:	8b 86 a0 00 00 00    	mov    0xa0(%esi),%eax
 * Shrink one step.
 */
tommy_inline void hashlin_shrink_step(tommy_hashlin* hashlin)
{
	/* shrink if less than 12.5% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_SHRINK
    284e:	8b 8e 9c 00 00 00    	mov    0x9c(%esi),%ecx
	while (i) {
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
			tommy_list_remove_existing(let_ptr, i);

			--hashlin->count;
    2854:	8d 50 ff             	lea    -0x1(%eax),%edx
 * Shrink one step.
 */
tommy_inline void hashlin_shrink_step(tommy_hashlin* hashlin)
{
	/* shrink if less than 12.5% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_SHRINK
    2857:	83 f9 02             	cmp    $0x2,%ecx
	while (i) {
		/* we first check if the hash matches, as in the same bucket we may have multiples hash values */
		if (i->key == hash && cmp(cmp_arg, i->data) == 0) {
			tommy_list_remove_existing(let_ptr, i);

			--hashlin->count;
    285a:	89 96 a0 00 00 00    	mov    %edx,0xa0(%esi)
 * Shrink one step.
 */
tommy_inline void hashlin_shrink_step(tommy_hashlin* hashlin)
{
	/* shrink if less than 12.5% full */
	if (hashlin->state != TOMMY_HASHLIN_STATE_SHRINK
    2860:	0f 84 74 01 00 00    	je     29da <tommy_hashlin_remove+0x23a>
		&& hashlin->count < hashlin->bucket_max / 8
    2866:	8b 86 84 00 00 00    	mov    0x84(%esi),%eax
    286c:	89 c7                	mov    %eax,%edi
    286e:	c1 ef 03             	shr    $0x3,%edi
    2871:	39 fa                	cmp    %edi,%edx
    2873:	0f 83 f8 00 00 00    	jae    2971 <tommy_hashlin_remove+0x1d1>
	) {
		/* avoid to shrink the first bucket */
		if (hashlin->bucket_bit > TOMMY_HASHLIN_BIT) {
    2879:	83 be 80 00 00 00 06 	cmpl   $0x6,0x80(%esi)
    2880:	0f 86 eb 00 00 00    	jbe    2971 <tommy_hashlin_remove+0x1d1>
			/* if we are stable, setup a new shrink state */
			/* otherwise continue with the already setup grow one */
			/* but in backward direction */
			if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
    2886:	85 c9                	test   %ecx,%ecx
    2888:	0f 85 7a 01 00 00    	jne    2a08 <tommy_hashlin_remove+0x268>
				/* set the lower size */
				hashlin->low_max = hashlin->bucket_max / 2;
				hashlin->low_mask = hashlin->bucket_mask / 2;
    288e:	8b 8e 88 00 00 00    	mov    0x88(%esi),%ecx
			/* if we are stable, setup a new shrink state */
			/* otherwise continue with the already setup grow one */
			/* but in backward direction */
			if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
				/* set the lower size */
				hashlin->low_max = hashlin->bucket_max / 2;
    2894:	d1 e8                	shr    %eax
    2896:	89 86 90 00 00 00    	mov    %eax,0x90(%esi)
				hashlin->low_mask = hashlin->bucket_mask / 2;

				/* start from the half going backward */
				hashlin->split = hashlin->low_max;
    289c:	89 86 98 00 00 00    	mov    %eax,0x98(%esi)
    28a2:	89 44 24 04          	mov    %eax,0x4(%esp)
			/* otherwise continue with the already setup grow one */
			/* but in backward direction */
			if (hashlin->state == TOMMY_HASHLIN_STATE_STABLE) {
				/* set the lower size */
				hashlin->low_max = hashlin->bucket_max / 2;
				hashlin->low_mask = hashlin->bucket_mask / 2;
    28a6:	d1 e9                	shr    %ecx
    28a8:	89 8e 94 00 00 00    	mov    %ecx,0x94(%esi)
				/* start from the half going backward */
				hashlin->split = hashlin->low_max;
			}

			/* start reallocation */
			hashlin->state = TOMMY_HASHLIN_STATE_SHRINK;
    28ae:	c7 86 9c 00 00 00 02 	movl   $0x2,0x9c(%esi)
    28b5:	00 00 00 
	}

	/* if we are shrinking */
	if (hashlin->state == TOMMY_HASHLIN_STATE_SHRINK) {
		/* compute the split target required to finish the reallocation before the next resize */
		unsigned split_target = 8 * hashlin->count;
    28b8:	c1 e2 03             	shl    $0x3,%edx
    28bb:	8d 2c 85 fc ff ff ff 	lea    -0x4(,%eax,4),%ebp
    28c2:	89 5c 24 0c          	mov    %ebx,0xc(%esp)
    28c6:	89 54 24 08          	mov    %edx,0x8(%esp)
    28ca:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    28d0:	8b 54 24 04          	mov    0x4(%esp),%edx
    28d4:	01 c2                	add    %eax,%edx

		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max > split_target) {
    28d6:	39 54 24 08          	cmp    %edx,0x8(%esp)
    28da:	0f 83 0f 01 00 00    	jae    29ef <tommy_hashlin_remove+0x24f>
			tommy_hashlin_node** split[2];

			/* go backward position */
			--hashlin->split;
    28e0:	83 e8 01             	sub    $0x1,%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    28e3:	83 f8 3f             	cmp    $0x3f,%eax
		/* reallocate buckets until the split target */
		while (hashlin->split + hashlin->low_max > split_target) {
			tommy_hashlin_node** split[2];

			/* go backward position */
			--hashlin->split;
    28e6:	89 86 98 00 00 00    	mov    %eax,0x98(%esi)
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    28ec:	0f 87 ce 00 00 00    	ja     29c0 <tommy_hashlin_remove+0x220>
		return &hashlin->bucket[0][pos];
    28f2:	8b 3e                	mov    (%esi),%edi
    28f4:	01 ef                	add    %ebp,%edi
			split[0] = tommy_hashlin_pos(hashlin, hashlin->split);

			/* get the high bucket */
			/* it's always in the second half, so we can index it directly */
			/* without calling tommy_hashlin_pos() */
			split[1] = &hashlin->bucket[hashlin->bucket_mac-1][hashlin->split];
    28f6:	8b 96 8c 00 00 00    	mov    0x8c(%esi),%edx
    28fc:	83 ea 01             	sub    $0x1,%edx
    28ff:	8b 0c 96             	mov    (%esi,%edx,4),%ecx
    2902:	8b 0c 29             	mov    (%ecx,%ebp,1),%ecx
{
	tommy_node* first_head;
	tommy_node* first_tail;
	tommy_node* second_head;

	if (tommy_list_empty(second)) {
    2905:	85 c9                	test   %ecx,%ecx
    2907:	74 1a                	je     2923 <tommy_hashlin_remove+0x183>
    2909:	8b 1f                	mov    (%edi),%ebx
		return;
	}

	if (tommy_list_empty(first)) {
    290b:	85 db                	test   %ebx,%ebx
    290d:	74 12                	je     2921 <tommy_hashlin_remove+0x181>
	tommy_node* head = tommy_list_head(list);

	if (!head)
		return 0;

	return head->prev;
    290f:	8b 7b 04             	mov    0x4(%ebx),%edi
    2912:	89 3c 24             	mov    %edi,(%esp)
	first_head = tommy_list_head(first);
	second_head = tommy_list_head(second);
	first_tail = tommy_list_tail(first);

	/* set the "circular" prev list */
	first_head->prev = second_head->prev;
    2915:	8b 79 04             	mov    0x4(%ecx),%edi
    2918:	89 7b 04             	mov    %edi,0x4(%ebx)
	second_head->prev = first_tail;
    291b:	8b 3c 24             	mov    (%esp),%edi
    291e:	89 79 04             	mov    %edi,0x4(%ecx)

	/* set the "0 terminated" next list */
	first_tail->next = second_head;
    2921:	89 0f                	mov    %ecx,(%edi)
    2923:	83 ed 04             	sub    $0x4,%ebp

			/* concat the high bucket into the low one */
			tommy_list_concat(split[0], split[1]);

			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
    2926:	85 c0                	test   %eax,%eax
    2928:	75 a6                	jne    28d0 <tommy_hashlin_remove+0x130>
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;

				/* shrink the hash size */
				--hashlin->bucket_bit;
    292a:	8b 86 80 00 00 00    	mov    0x80(%esi),%eax
    2930:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
				hashlin->bucket_mask = hashlin->bucket_max - 1;

				/* free the last segment */
				--hashlin->bucket_mac;
				tommy_free(hashlin->bucket[hashlin->bucket_mac]);
    2934:	83 ec 0c             	sub    $0xc,%esp
			/* concat the high bucket into the low one */
			tommy_list_concat(split[0], split[1]);

			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;
    2937:	c7 86 9c 00 00 00 00 	movl   $0x0,0x9c(%esi)
    293e:	00 00 00 
				--hashlin->bucket_bit;
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
				hashlin->bucket_mask = hashlin->bucket_max - 1;

				/* free the last segment */
				--hashlin->bucket_mac;
    2941:	89 96 8c 00 00 00    	mov    %edx,0x8c(%esi)
			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;

				/* shrink the hash size */
				--hashlin->bucket_bit;
    2947:	8d 48 ff             	lea    -0x1(%eax),%ecx
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
    294a:	b8 01 00 00 00       	mov    $0x1,%eax
    294f:	d3 e0                	shl    %cl,%eax
			/* if we have finished, clean up and change the state */
			if (hashlin->split == 0) {
				hashlin->state = TOMMY_HASHLIN_STATE_STABLE;

				/* shrink the hash size */
				--hashlin->bucket_bit;
    2951:	89 8e 80 00 00 00    	mov    %ecx,0x80(%esi)
				hashlin->bucket_max = 1 << hashlin->bucket_bit;
    2957:	89 86 84 00 00 00    	mov    %eax,0x84(%esi)
				hashlin->bucket_mask = hashlin->bucket_max - 1;
    295d:	83 e8 01             	sub    $0x1,%eax
    2960:	89 86 88 00 00 00    	mov    %eax,0x88(%esi)

				/* free the last segment */
				--hashlin->bucket_mac;
				tommy_free(hashlin->bucket[hashlin->bucket_mac]);
    2966:	ff 34 96             	pushl  (%esi,%edx,4)
    2969:	e8 fc ff ff ff       	call   296a <tommy_hashlin_remove+0x1ca>
    296e:	83 c4 10             	add    $0x10,%esp

			--hashlin->count;

			hashlin_shrink_step(hashlin);

			return i->data;
    2971:	8b 43 08             	mov    0x8(%ebx),%eax
		}
		i = i->next;
	}

	return 0;
}
    2974:	83 c4 1c             	add    $0x1c,%esp
    2977:	5b                   	pop    %ebx
    2978:	5e                   	pop    %esi
    2979:	5f                   	pop    %edi
    297a:	5d                   	pop    %ebp
    297b:	c3                   	ret    
    297c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
			return tommy_hashlin_pos(hashlin, pos);
		}
	}

	/* otherwise operates normally */
	pos = hash & hashlin->bucket_mask;
    2980:	8b 86 88 00 00 00    	mov    0x88(%esi),%eax
    2986:	21 f8                	and    %edi,%eax
tommy_inline tommy_hashlin_node** tommy_hashlin_pos(tommy_hashlin* hashlin, tommy_hash_t pos)
{  
	unsigned bsr;  
 
	/* special case for the first bucket */
	if (pos < (1 << TOMMY_HASHLIN_BIT)) {
    2988:	83 f8 3f             	cmp    $0x3f,%eax
    298b:	0f 86 49 fe ff ff    	jbe    27da <tommy_hashlin_remove+0x3a>
    2991:	0f bd c8             	bsr    %eax,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    2994:	bb 01 00 00 00       	mov    $0x1,%ebx
    2999:	d3 e3                	shl    %cl,%ebx

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    299b:	8b 4c 8e ec          	mov    -0x14(%esi,%ecx,4),%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    299f:	29 d8                	sub    %ebx,%eax

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    29a1:	8d 2c 81             	lea    (%ecx,%eax,4),%ebp
    29a4:	e9 36 fe ff ff       	jmp    27df <tommy_hashlin_remove+0x3f>
    29a9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
		}
		i = i->next;
	}

	return 0;
}
    29b0:	83 c4 1c             	add    $0x1c,%esp
			return i->data;
		}
		i = i->next;
	}

	return 0;
    29b3:	31 c0                	xor    %eax,%eax
}
    29b5:	5b                   	pop    %ebx
    29b6:	5e                   	pop    %esi
    29b7:	5f                   	pop    %edi
    29b8:	5d                   	pop    %ebp
    29b9:	c3                   	ret    
    29ba:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    29c0:	0f bd c8             	bsr    %eax,%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    29c3:	ba 01 00 00 00       	mov    $0x1,%edx
    29c8:	89 c7                	mov    %eax,%edi
    29ca:	d3 e2                	shl    %cl,%edx

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    29cc:	8b 4c 8e ec          	mov    -0x14(%esi,%ecx,4),%ecx

	/* get the highest bit set */
	bsr = tommy_ilog2_u32(pos);

	/* clear the highest bit */
	pos -= 1 << bsr;
    29d0:	29 d7                	sub    %edx,%edi

	return &hashlin->bucket[bsr - TOMMY_HASHLIN_BIT + 1][pos];
    29d2:	8d 3c b9             	lea    (%ecx,%edi,4),%edi
    29d5:	e9 1c ff ff ff       	jmp    28f6 <tommy_hashlin_remove+0x156>
    29da:	8b 86 90 00 00 00    	mov    0x90(%esi),%eax
    29e0:	89 44 24 04          	mov    %eax,0x4(%esp)
    29e4:	8b 86 98 00 00 00    	mov    0x98(%esi),%eax
    29ea:	e9 c9 fe ff ff       	jmp    28b8 <tommy_hashlin_remove+0x118>
    29ef:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
    29f3:	e9 79 ff ff ff       	jmp    2971 <tommy_hashlin_remove+0x1d1>
		head->prev = node->prev; /* the last */
	}

	/* remove from the "0 terminated" next list */
	if (head == node) {
		*list = node->next; /* the new head, in case 0 */
    29f8:	89 45 00             	mov    %eax,0x0(%ebp)
    29fb:	e9 48 fe ff ff       	jmp    2848 <tommy_hashlin_remove+0xa8>

	/* remove from the "circular" prev list */
	if (node->next) { 
		node->next->prev = node->prev;
	} else {
		head->prev = node->prev; /* the last */
    2a00:	89 4a 04             	mov    %ecx,0x4(%edx)
    2a03:	e9 33 fe ff ff       	jmp    283b <tommy_hashlin_remove+0x9b>
    2a08:	8b 86 90 00 00 00    	mov    0x90(%esi),%eax
    2a0e:	89 44 24 04          	mov    %eax,0x4(%esp)
    2a12:	8b 86 98 00 00 00    	mov    0x98(%esi),%eax
    2a18:	e9 91 fe ff ff       	jmp    28ae <tommy_hashlin_remove+0x10e>
    2a1d:	8d 76 00             	lea    0x0(%esi),%esi

00002a20 <tommy_hashlin_memory_usage>:

	return 0;
}

tommy_size_t tommy_hashlin_memory_usage(tommy_hashlin* hashlin)
{
    2a20:	8b 44 24 04          	mov    0x4(%esp),%eax
	return hashlin->bucket_max * (tommy_size_t)sizeof(hashlin->bucket[0][0])
		+ hashlin->count * (tommy_size_t)sizeof(tommy_hashlin_node);
    2a24:	8b 90 a0 00 00 00    	mov    0xa0(%eax),%edx
    2a2a:	8b 80 84 00 00 00    	mov    0x84(%eax),%eax
    2a30:	8d 04 90             	lea    (%eax,%edx,4),%eax
	return 0;
}

tommy_size_t tommy_hashlin_memory_usage(tommy_hashlin* hashlin)
{
	return hashlin->bucket_max * (tommy_size_t)sizeof(hashlin->bucket[0][0])
    2a33:	c1 e0 02             	shl    $0x2,%eax
		+ hashlin->count * (tommy_size_t)sizeof(tommy_hashlin_node);
}
    2a36:	c3                   	ret    

Disassembly of section .text.unlikely:

00000000 <tommy_array_ref.part.11>:
/**
 * Gets a reference of the element at the specified position.
 * You must be sure that space for this position is already
 * allocated calling tommy_array_grow().
 */
tommy_inline void** tommy_array_ref(tommy_array* array, unsigned pos)
   0:	83 ec 0c             	sub    $0xc,%esp
{
	unsigned bsr;  

	assert(pos < array->size);
   3:	68 da 00 00 00       	push   $0xda
   8:	6a 61                	push   $0x61
   a:	68 00 00 00 00       	push   $0x0
   f:	68 0d 00 00 00       	push   $0xd
  14:	e8 fc ff ff ff       	call   15 <tommy_array_ref.part.11+0x15>
