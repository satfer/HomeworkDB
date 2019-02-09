#pragma once

// char(1) 和char(N)不一样, 无结尾0
// data同CHAR(10)处理, 用char[10+1]存, 字典序并不会变, 有结尾0
// lineitem我换了列的顺序



#include <cstdio>
#include <cstring>

int dbgen() {
	FILE *fpr, *fpDB, *fpIndex, *fpSchema;
	char c;
	int i;
	fpos_t fpos;

	// NATION
	if (fopen_s(&fpr, "../instance/nation.tbl", "r") || fopen_s(&fpDB, "../instance/nation.db", "wb") || \
		fopen_s(&fpIndex, "../instance/nation.index", "wb") || fopen_s(&fpSchema, "../instance/nation.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "N_NATIONKEY INTEGER\n");
		fprintf(fpSchema, "N_NAME CHAR(25)\n");
		fprintf(fpSchema, "N_REGIONKEY INTEGER\n");
		fprintf(fpSchema, "N_COMMENT VARCHAR(152)\n");

		int N_NATIONKEY;
		char N_NAME[25 + 1];
		int N_REGIONKEY;
		char N_COMMENT[152 + 1];

		// 这是feof的陷阱, 正好读完的时候, "fp->flag仍然没有被置为_IOEOF", 顺着读会多一行.
		// 只能这样: 先在循环外读第一个词, 然后每次在循环末尾读下一次的第一个词
		fscanf_s(fpr, "%d|", &N_NATIONKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				N_NAME[i] = c;
			}
			N_NAME[i] = '\0';

			fscanf_s(fpr, "%d|", &N_REGIONKEY);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 152; ++i) {
				N_COMMENT[i] = c;
			}
			N_COMMENT[i] = '\0';

			// 输出看看
			// printf("%d|%s|%d|%s|\n", N_NATIONKEY, N_NAME, N_REGIONKEY, N_COMMENT);

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&N_NATIONKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&N_NATIONKEY, sizeof(int), 1, fpDB);
			fwrite(N_NAME, sizeof(char), 25 + 1, fpDB);
			fwrite(&N_REGIONKEY, sizeof(int), 1, fpDB);
			fwrite(N_COMMENT, sizeof(char), strlen(N_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写

			

			// feof 陷阱
			fscanf_s(fpr, "%d|", &N_NATIONKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// REGION
	if (fopen_s(&fpr, "../instance/region.tbl", "r") || fopen_s(&fpDB, "../instance/region.db", "wb") || \
		fopen_s(&fpIndex, "../instance/region.index", "wb") || fopen_s(&fpSchema, "../instance/region.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "R_REGIONKEY INTEGER\n");
		fprintf(fpSchema, "R_NAME CHAR(25)\n");
		fprintf(fpSchema, "R_COMMENT VARCHAR(152)\n");

		int R_REGIONKEY;
		char R_NAME[25 + 1];
		char R_COMMENT[152 + 1];

		fscanf_s(fpr, "%d|", &R_REGIONKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				R_NAME[i] = c;
			}
			R_NAME[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 152; ++i) {
				R_COMMENT[i] = c;
			}
			R_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&R_REGIONKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&R_REGIONKEY, sizeof(int), 1, fpDB);
			fwrite(R_NAME, sizeof(char), 25 + 1, fpDB);
			fwrite(R_COMMENT, sizeof(char), strlen(R_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写

			

			fscanf_s(fpr, "%d|", &R_REGIONKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// PART
	if (fopen_s(&fpr, "../instance/part.tbl", "r") || fopen_s(&fpDB, "../instance/part.db", "wb") || \
		fopen_s(&fpIndex, "../instance/part.index", "wb") || fopen_s(&fpSchema, "../instance/part.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "P_PARTKEY INTEGER\n");
		fprintf(fpSchema, "P_NAME VARCHAR(55)\n");
		fprintf(fpSchema, "P_MFGR CHAR(25)\n");
		fprintf(fpSchema, "P_BRAND CHAR(10)\n");
		fprintf(fpSchema, "P_TYPE VARCHAR(25)\n");
		fprintf(fpSchema, "P_SIZE INTEGER\n");
		fprintf(fpSchema, "P_CONTAINER CHAR(10)\n");
		fprintf(fpSchema, "P_RETAILPRICE DOUBLE\n");
		fprintf(fpSchema, "P_COMMENT VARCHAR(23)\n");

		int P_PARTKEY;
		char P_NAME[55 + 1];
		char P_MFGR[25 + 1];
		char P_BRAND[10 + 1];
		char P_TYPE[25 + 1];
		int P_SIZE;
		char P_CONTAINER[10 + 1];
		double P_RETAILPRICE;
		char P_COMMENT[23 + 1];

		fscanf_s(fpr, "%d|", &P_PARTKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			for (i = 0; (c = fgetc(fpr)) != '|' && i < 55; ++i) {
				P_NAME[i] = c;
			}
			P_NAME[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				P_MFGR[i] = c;
			}
			P_MFGR[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				P_BRAND[i] = c;
			}
			P_BRAND[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				P_TYPE[i] = c;
			}
			P_TYPE[i] = '\0';

			fscanf_s(fpr, "%d|", &P_SIZE);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				P_CONTAINER[i] = c;
			}
			P_CONTAINER[i] = '\0';

			fscanf_s(fpr, "%lf|", &P_RETAILPRICE);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 23; ++i) {
				P_COMMENT[i] = c;
			}
			P_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&P_PARTKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&P_PARTKEY, sizeof(int), 1, fpDB);
			fwrite(P_NAME, sizeof(char), strlen(P_NAME) + 1, fpDB);
			fwrite(P_MFGR, sizeof(char), 25 + 1, fpDB);
			fwrite(P_BRAND, sizeof(char), 10 + 1, fpDB);
			fwrite(P_TYPE, sizeof(char), strlen(P_TYPE) + 1, fpDB);
			fwrite(&P_SIZE, sizeof(int), 1, fpDB);
			fwrite(P_CONTAINER, sizeof(char), 10 + 1, fpDB);
			fwrite(&P_RETAILPRICE, sizeof(double), 1, fpDB);
			fwrite(P_COMMENT, sizeof(char), strlen(P_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写



			fscanf_s(fpr, "%d|", &P_PARTKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// SUPPLIER
	if (fopen_s(&fpr, "../instance/supplier.tbl", "r") || fopen_s(&fpDB, "../instance/supplier.db", "wb") || \
		fopen_s(&fpIndex, "../instance/supplier.index", "wb") || fopen_s(&fpSchema, "../instance/supplier.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "S_SUPPKEY INTEGER\n");
		fprintf(fpSchema, "S_NAME CHAR(25)\n");
		fprintf(fpSchema, "S_ADDRESS VARCHAR(40)\n");
		fprintf(fpSchema, "S_NATIONKEY INTEGER\n");
		fprintf(fpSchema, "S_PHONE CHAR(15)\n");
		fprintf(fpSchema, "S_ACCTBAL DOUBLE\n");
		fprintf(fpSchema, "S_COMMENT VARCHAR(101)\n");

		int S_SUPPKEY;
		char S_NAME[25 + 1];
		char S_ADDRESS[40 + 1];
		int S_NATIONKEY;
		char S_PHONE[15 + 1];
		double S_ACCTBAL;
		char S_COMMENT[101 + 1];

		fscanf_s(fpr, "%d|", &S_SUPPKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				S_NAME[i] = c;
			}
			S_NAME[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 40; ++i) {
				S_ADDRESS[i] = c;
			}
			S_ADDRESS[i] = '\0';

			fscanf_s(fpr, "%d|", &S_NATIONKEY);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 15; ++i) {
				S_PHONE[i] = c;
			}
			S_PHONE[i] = '\0';

			fscanf_s(fpr, "%lf|", &S_ACCTBAL);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 101; ++i) {
				S_COMMENT[i] = c;
			}
			S_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&S_SUPPKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&S_SUPPKEY, sizeof(int), 1, fpDB);
			fwrite(S_NAME, sizeof(char), 25 + 1, fpDB);
			fwrite(S_ADDRESS, sizeof(char), strlen(S_ADDRESS) + 1, fpDB);
			fwrite(&S_NATIONKEY, sizeof(int), 1, fpDB);
			fwrite(S_PHONE, sizeof(char), 15 + 1, fpDB);
			fwrite(&S_ACCTBAL, sizeof(double), 1, fpDB);
			fwrite(S_COMMENT, sizeof(char), strlen(S_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写



			fscanf_s(fpr, "%d|", &S_SUPPKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// PARTSUPP
	// 我这多了个记录行数的列
	if (fopen_s(&fpr, "../instance/partsupp.tbl", "r") || fopen_s(&fpDB, "../instance/partsupp.db", "wb") || \
		fopen_s(&fpIndex, "../instance/partsupp.index", "wb") || fopen_s(&fpSchema, "../instance/partsupp.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "PS_PSKEY INTEGER\n"); //自己生成的, 拥挤记录行数, 一般的数据库是隐藏的
		fprintf(fpSchema, "PS_PARTKEY INTEGER\n");
		fprintf(fpSchema, "PS_SUPPKEY INTEGER\n");
		fprintf(fpSchema, "PS_AVAILQTY INTEGER\n");
		fprintf(fpSchema, "PS_SUPPLYCOST DOUBLE\n");
		fprintf(fpSchema, "PS_COMMENT VARCHAR(199)\n");

		int PS_PSKEY = -1;
		int PS_PARTKEY, PS_SUPPKEY, PS_AVAILQTY;
		double PS_SUPPLYCOST;
		char PS_COMMENT[199 + 1];

		fscanf_s(fpr, "%d|", &PS_PARTKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			fscanf_s(fpr, "%d|", &PS_SUPPKEY);

			fscanf_s(fpr, "%d|", &PS_AVAILQTY);

			fscanf_s(fpr, "%lf|", &PS_SUPPLYCOST);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 199; ++i) {
				PS_COMMENT[i] = c;
			}
			PS_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			++PS_PSKEY;
			fgetpos(fpDB, &fpos);
			fwrite(&PS_PSKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&PS_PSKEY, sizeof(int), 1, fpDB);
			fwrite(&PS_PARTKEY, sizeof(int), 1, fpDB);
			fwrite(&PS_SUPPKEY, sizeof(int), 1, fpDB);
			fwrite(&PS_AVAILQTY, sizeof(int), 1, fpDB);
			fwrite(&PS_SUPPLYCOST, sizeof(double), 1, fpDB);
			fwrite(PS_COMMENT, sizeof(char), strlen(PS_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写



			fscanf_s(fpr, "%d|", &PS_PARTKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// CUSTOMER
	if (fopen_s(&fpr, "../instance/customer.tbl", "r") || fopen_s(&fpDB, "../instance/customer.db", "wb") || \
		fopen_s(&fpIndex, "../instance/customer.index", "wb") || fopen_s(&fpSchema, "../instance/customer.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "C_CUSTKEY INTEGER\n");
		fprintf(fpSchema, "C_NAME VARCHAR(25)\n");
		fprintf(fpSchema, "C_ADDRESS VARCHAR(40)\n");
		fprintf(fpSchema, "C_NATIONKEY INTEGER\n");
		fprintf(fpSchema, "C_PHONE CHAR(15)\n");
		fprintf(fpSchema, "C_ACCTBAL DOUBLE\n");
		fprintf(fpSchema, "C_MKTSEGMENT CHAR(10)\n");
		fprintf(fpSchema, "C_COMMENT VARCHAR(117)\n");

		int C_CUSTKEY;
		char C_NAME[25 + 1];
		char C_ADDRESS[40 + 1];
		int C_NATIONKEY;
		char C_PHONE[15 + 1];
		double C_ACCTBAL;
		char C_MKTSEGMENT[10 + 1];
		char C_COMMENT[117 + 1];

		fscanf_s(fpr, "%d|", &C_CUSTKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				C_NAME[i] = c;
			}
			C_NAME[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 40; ++i) {
				C_ADDRESS[i] = c;
			}
			C_ADDRESS[i] = '\0';

			fscanf_s(fpr, "%d|", &C_NATIONKEY);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 15; ++i) {
				C_PHONE[i] = c;
			}
			C_PHONE[i] = '\0';

			fscanf_s(fpr, "%lf|", &C_ACCTBAL);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				C_MKTSEGMENT[i] = c;
			}
			C_MKTSEGMENT[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 117; ++i) {
				C_COMMENT[i] = c;
			}
			C_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&C_CUSTKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&C_CUSTKEY, sizeof(int), 1, fpDB);
			fwrite(C_NAME, sizeof(char), strlen(C_NAME) + 1, fpDB);
			fwrite(C_ADDRESS, sizeof(char), strlen(C_ADDRESS) + 1, fpDB);
			fwrite(&C_NATIONKEY, sizeof(int), 1, fpDB);
			fwrite(C_PHONE, sizeof(char), 15 + 1, fpDB);
			fwrite(&C_ACCTBAL, sizeof(double), 1, fpDB);
			fwrite(C_MKTSEGMENT, sizeof(char), 10 + 1, fpDB);
			fwrite(C_COMMENT, sizeof(char), strlen(C_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写



			fscanf_s(fpr, "%d|", &C_CUSTKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// ORDERS
	if (fopen_s(&fpr, "../instance/orders.tbl", "r") || fopen_s(&fpDB, "../instance/orders.db", "wb") || \
		fopen_s(&fpIndex, "../instance/orders.index", "wb") || fopen_s(&fpSchema, "../instance/orders.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "O_ORDERKEY INTEGER\n");
		fprintf(fpSchema, "O_CUSTKEY INTEGER\n");
		fprintf(fpSchema, "O_ORDERSTATUS CHAR(1)\n");
		fprintf(fpSchema, "O_TOTALPRICE DOUBLE\n");
		fprintf(fpSchema, "O_ORDERDATE DATE\n");
		fprintf(fpSchema, "O_ORDERPRIORITY CHAR(15)\n");
		fprintf(fpSchema, "O_CLERK CHAR(15)\n");
		fprintf(fpSchema, "O_SHIPPRIORITY INTEGER\n");
		fprintf(fpSchema, "O_COMMENT VARCHAR(79)\n");

		int O_ORDERKEY, O_CUSTKEY;
		char O_ORDERSTATUS;
		double O_TOTALPRICE;
		char O_ORDERDATE[10 + 1]; // DATE
		char O_ORDERPRIORITY[15 + 1];
		char O_CLERK[15 + 1];
		int O_SHIPPRIORITY;
		char O_COMMENT[79 + 1];

		fscanf_s(fpr, "%d|", &O_ORDERKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			fscanf_s(fpr, "%d|", &O_ORDERKEY);

			O_ORDERSTATUS = fgetc(fpr); fgetc(fpr); // 吃'\'

			fscanf_s(fpr, "%lf|", &O_TOTALPRICE);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				O_ORDERDATE[i] = c;
			}
			O_ORDERDATE[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 15; ++i) {
				O_ORDERPRIORITY[i] = c;
			}
			O_ORDERPRIORITY[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 15; ++i) {
				O_CLERK[i] = c;
			}
			O_CLERK[i] = '\0';

			fscanf_s(fpr, "%d|", &O_SHIPPRIORITY);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 79; ++i) {
				O_COMMENT[i] = c;
			}
			O_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&O_ORDERKEY, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&O_ORDERKEY, sizeof(int), 1, fpDB);
			fwrite(&O_CUSTKEY, sizeof(int), 1, fpDB);
			fwrite(&O_ORDERSTATUS, sizeof(char), 1, fpDB);
			fwrite(&O_TOTALPRICE, sizeof(double), 1, fpDB);
			fwrite(O_ORDERDATE, sizeof(char), 10 + 1, fpDB); // DATE
			fwrite(O_ORDERPRIORITY, sizeof(char), 15 + 1, fpDB);
			fwrite(O_CLERK, sizeof(char), 15 + 1, fpDB);
			fwrite(&O_SHIPPRIORITY, sizeof(int), 1, fpDB);
			fwrite(O_COMMENT, sizeof(char), strlen(O_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写



			fscanf_s(fpr, "%d|", &O_ORDERKEY);
		} // while (!feof(fpr))
	} // openFileFailed else

	// LINEITEM
	// 我把L_LINENUMBER换了下位置 放在最前面了, (也没太大必要 方便点点
	if (fopen_s(&fpr, "../instance/lineitem.tbl", "r") || fopen_s(&fpDB, "../instance/lineitem.db", "wb") || \
		fopen_s(&fpIndex, "../instance/lineitem.index", "wb") || fopen_s(&fpSchema, "../instance/lineitem.schema", "w")) {
		throw("Open file failed.");
	}
	else {
		// 存Schema
		fprintf(fpSchema, "L_LINENUMBER INTEGER\n");
		fprintf(fpSchema, "L_ORDERKEY INTEGER\n");
		fprintf(fpSchema, "L_PARTKEY INTEGER\n");
		fprintf(fpSchema, "L_SUPPKEY INTEGER\n");
		fprintf(fpSchema, "L_QUANTITY DOUBLE\n");
		fprintf(fpSchema, "L_EXTENDEDPRICE DOUBLE\n");
		fprintf(fpSchema, "L_DISCOUNT DOUBLE\n");
		fprintf(fpSchema, "L_TAX DOUBLE\n");
		fprintf(fpSchema, "L_RETURNFLAG CHAR(1)\n");
		fprintf(fpSchema, "L_LINESTATUS CHAR(1)\n");
		fprintf(fpSchema, "L_SHIPDATE DATE\n");
		fprintf(fpSchema, "L_COMMITDATE DATE\n");
		fprintf(fpSchema, "L_RECEIPTDATE DATE\n");
		fprintf(fpSchema, "L_SHIPINSTRUCT CHAR(25)\n");
		fprintf(fpSchema, "L_SHIPMODE CHAR(10)\n");
		fprintf(fpSchema, "L_COMMENT VARCHAR(44)\n");

		int L_LINENUMBER;
		int L_ORDERKEY, L_PARTKEY, L_SUPPKEY;
		double L_QUANTITY, L_EXTENDEDPRICE, L_DISCOUNT, L_TAX;
		char L_RETURNFLAG, L_LINESTATUS;
		char L_SHIPDATE[10 + 1], L_COMMITDATE[10 + 1], L_RECEIPTDATE[10 + 1]; // DATE
		char L_SHIPINSTRUCT[25 + 1];
		char L_SHIPMODE[10 + 1];
		char L_COMMENT[44 + 1];

		fscanf_s(fpr, "%d|", &L_ORDERKEY);
		while (!feof(fpr)) {
			// 连着循环前一行 或 循环最后一行, 是读文件
			fscanf_s(fpr, "%d|", &L_PARTKEY);
			fscanf_s(fpr, "%d|", &L_SUPPKEY);
			fscanf_s(fpr, "%d|", &L_LINENUMBER);

			fscanf_s(fpr, "%lf|", &L_QUANTITY);
			fscanf_s(fpr, "%lf|", &L_EXTENDEDPRICE);
			fscanf_s(fpr, "%lf|", &L_DISCOUNT);
			fscanf_s(fpr, "%lf|", &L_TAX);

			L_RETURNFLAG = fgetc(fpr); fgetc(fpr);
			L_LINESTATUS = fgetc(fpr); fgetc(fpr);

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				L_SHIPDATE[i] = c;
			}
			L_SHIPDATE[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				L_COMMITDATE[i] = c;
			}
			L_COMMITDATE[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				L_RECEIPTDATE[i] = c;
			}
			L_RECEIPTDATE[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 25; ++i) {
				L_SHIPINSTRUCT[i] = c;
			}
			L_SHIPINSTRUCT[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 10; ++i) {
				L_SHIPMODE[i] = c;
			}
			L_SHIPMODE[i] = '\0';

			for (i = 0; (c = fgetc(fpr)) != '|' && i < 44; ++i) {
				L_COMMENT[i] = c;
			}
			L_COMMENT[i] = '\0';

			// 存主索引(标号的索引), 用顺序表
			fgetpos(fpDB, &fpos);
			fwrite(&L_LINENUMBER, sizeof(int), 1, fpIndex);
			fwrite(&fpos, sizeof(fpos_t), 1, fpIndex);

			// 存db
			fwrite(&L_LINENUMBER, sizeof(int), 1, fpDB);
			fwrite(&L_ORDERKEY, sizeof(int), 1, fpDB);
			fwrite(&L_PARTKEY, sizeof(int), 1, fpDB);
			fwrite(&L_SUPPKEY, sizeof(int), 1, fpDB);
			fwrite(&L_QUANTITY, sizeof(double), 1, fpDB);
			fwrite(&L_EXTENDEDPRICE, sizeof(double), 1, fpDB);
			fwrite(&L_DISCOUNT, sizeof(double), 1, fpDB);
			fwrite(&L_TAX, sizeof(double), 1, fpDB);
			fwrite(&L_RETURNFLAG, sizeof(char), 1, fpDB);
			fwrite(&L_LINESTATUS, sizeof(char), 1, fpDB);
			fwrite(L_SHIPDATE, sizeof(char), 10 + 1, fpDB); // DATE
			fwrite(L_COMMITDATE, sizeof(char), 10 + 1, fpDB); // DATE
			fwrite(L_RECEIPTDATE, sizeof(char), 10 + 1, fpDB); // DATE
			fwrite(L_SHIPINSTRUCT, sizeof(char), 25 + 1, fpDB);
			fwrite(L_SHIPMODE, sizeof(char), 10 + 1, fpDB);
			fwrite(L_COMMENT, sizeof(char), strlen(L_COMMENT) + 1, fpDB); //VARCHAR 末尾的'\0'一起写



			fscanf_s(fpr, "%d|", &L_ORDERKEY);
		} // while (!feof(fpr))
	} // openFileFailed else
} // dbgen()