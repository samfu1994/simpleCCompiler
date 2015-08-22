int errorCount = 0;
int testArr1[100];
int testArr2[100][100];
int ttttt = -1;
struct asdasd{
	int a;int b;
}aaa;

struct asdasd bbb;
int gaafucktestnum = 0;
int gEightQueen[8] = {0};
int gCount = 0;
int feb2a[1000] = {1,1,0,0,0,0,0,0,0};

int gcd(int x,int y)
{
	if(!y)
		return x;
	return gcd(y,x%y);
}
int relopTest(){
	int i = 1;
	i = (((((i+i)+(i+i))+((i+i)+(i+i)))+(((i+i)+(i+i))+((i+i)+(i+i))))+((((i+i)+(i+i))+((i+i)+(i+i)))+(((i+i)+(i+i))+((i+i)+(i+i)))))+(((((i+i)+(i+i))+((i+i)+(i+i)))+(((i+i)+(i+i))+((i+i)+(i+i))))+((((i+i)+(i+i))+((i+i)+(i+i)))+(((i+i)+(i+i))+((i+i)+(i+i)))));
	if(i != 64)
		++errorCount;
	if(1 > 2)
		++errorCount;
	if(!(2 >= 2))
		++errorCount;
	if(2 > 2)
		++errorCount;
	if(2 < 1)
		++errorCount;
	if(2 <= 1)
		++errorCount;
	if(!(2 <= 2))
		++errorCount;
	if((2 < 2))
		++errorCount;

}


int map[8][8] =
{ //0,1,2,3,4,5,6,7
	0,1,1,1,0,0,0,0, //0
	0,0,0,0,1,0,0,0, //1
	1,1,1,0,1,0,0,0, //2
	0,0,1,0,1,0,0,0, //3
	0,0,1,0,1,0,0,0, //4
	0,0,1,0,1,0,0,0, //5
	0,0,1,0,1,1,1,1, //6
	0,0,1,0,0,0,0,0, //7
};
int pathx[8];
int pathy[8];
int visited[8][8];

int dfs(int x,int y){
	if(x > 7 || x < 0)
		return 0;
	if(y > 7 || y < 0)
		return 0;
	if(visited[y][x])
		return 0;
	if(x == 7 && y == 7)
		return 1;
	if(map[y][x] == 1)
		return 0;
	visited[y][x] = 1;
	if(dfs(x+1,y)){
		write(x);
		write(y);
		write(-1);
		visited[y][x] = 0;
		return 1;
	}
	if(dfs(x-1,y)){
		write(x);
		write(y);
		write(-1);
		visited[y][x] = 0;
		return 1;
	}
	if(dfs(x,y+1)){
		write(x);
		write(y);
		write(-1);
		visited[y][x] = 0;
		return 1;
	}
	if(dfs(x,y-1)){
		write(x);
		write(y);
		write(-1);
		visited[y][x] = 0;
		return 1;
	}
	return 0;
}

int feb2(int i){
	if(feb2a[i])
		return feb2a[i];
	feb2a[i] = feb2(i-1) + feb2(i-2);
	return feb2a[i];
}

//////////////////////////////////////
int print()
{
	int outer;
	int inner;
	for(outer = 0; outer <8; ++outer){
		write(gEightQueen[outer]);
	}
    write(10000);
    return 0;
}

int check_pos_valid(int loop, int value)
{
	int index;
	int data;

	for(index = 0; index < loop; ++index){
		data = gEightQueen[index];

		if(value == data)
			return 0;

		if((index + data) == (loop + value))
			return 0;

		if((index - data) == (loop - value))
			return 0;
	}

	return 1;
}



int eight_queen(int index)
{
	int loop;
	for(loop = 0; loop < 8; ++loop){
		if(check_pos_valid(index, loop)){
			gEightQueen[index] = loop;
			if(7 == index){
				++gCount;
				print();
			    gEightQueen[index] = 0;
				return 0;
			}
			eight_queen(index + 1);
			gEightQueen[index] = 0;
		}
	}
	return 0;
}
/////////////////////////////////////////

int structTest(){
	struct localStruct{
		int a,b;
		int c;
	}la;
	struct localStruct lb;
	// global struct
	aaa.a = 10;
	aaa.b = 200;
	bbb.b = aaa.a;
	bbb.a = aaa.b;
	aaa.a += 1;
	++aaa.a;
	if(aaa.a != 12)
		++errorCount;
	if(aaa.b != 200)
		++errorCount;
	if(bbb.a != 200)
		++errorCount;
	if(bbb.b != 10)
		++errorCount;
	// local struct
	la.a = 10;
	la.b = 200;
	lb.b = la.a;
	lb.a = la.b;
	la.a += 1;
	++la.a;
	if(la.a != 12)
		++errorCount;
	if(la.b != 200)
		++errorCount;
	if(lb.a != 200)
		++errorCount;
	if(lb.b != 10)
		++errorCount;
	bbb = aaa;
	la = lb;
	return 0;
}

int breakContinueTest(){
	int i = 0;
	int j = 0;
	for(i = 0;  ; ++i){
		if(i % 2)
			continue;
		if(i > 100)
			break;
		j += i;
	}
	if(j != 2550)
		++errorCount;
	return 0;
}

int callTest(int i){
	int arr [1000];
	if(i == 0)
		return 1;
	if(i == 1)
		return 1;
	return callTest(i-1)+callTest(i-2);
}

int logicArithmaticTest(){
	int i = 0;
	int j = 0;
	i = (i == j);
	if(i != 1)
		++errorCount;
	j = !(i == j);
	if(j != 1)
		++errorCount;
	j = 10;
	i = -1;
	i = (i < j);
	if(i != 1)
		++errorCount;
	i = 1;
	if(i){
	}else{
		++errorCount;
	}
	if(1){
	}else{
		++errorCount;
	}
	j = 0;
	if(j){
		++errorCount;
	}else{
	}
	if(j){
		++errorCount;
	}
	if(j)
		++errorCount;
	j = 0;
	i = 10;
	for(;i; --i){
		j = j + i;
	}
	if(j != 55)
		++errorCount;
	i = 0;

	i = 0; j = 1;
	if(((i +1) != 0) && (j != 0) ) {
	}else{
		++errorCount;
	}
	if((i!=0) && (j!=0)){
		++errorCount;
	}
	if((i!=0) || (j!=0)){
	}else{
		++errorCount;
	}

	i = 0; j = 1;
	if((i +1) && j ) {

	}else{
		++errorCount;
	}
	if(i && j){
		++errorCount;
	}
	if(i || j){
	}else{
		++errorCount;
	}

	return 0;
}

int arrayTest(){
	int tarr1[100];
	int tarr2[100][100];
	int i,j;
	for(i = 0; i < 100; ++i){
		testArr1[i] = i;
		tarr1[i] = i;
	}
	for(i = 0; i < 100; ++i){
		if(testArr1[i]!= i)
			++errorCount;
		if(tarr1[i]!= i)
			++errorCount;
	}
	return 0;
}

int scopeVariableTest(){
	int i = 0;
	int jj[100];
	jj[10] = 1001;
	++i;
	if(i != 1)
		++errorCount;
	{
		int i = 10;
		++i;
		if(i != 11)
			++errorCount;
		{
			int i = 20;
			++i;
			if(i != 21)
				++errorCount;
			{
				int i = 30;
				i = i * 10;
				if(i != 300)
					++errorCount;
			}
			++i;
			if(i != 22)
				++errorCount;
		}
		++i;
		if(i!=12)
			++errorCount;
	}
	++i;
	if(i!=2)
		++errorCount;
}

int arithmaticTest(){
	int i, j;
	i = 100;
	j = 31;
	if(i % j != 7)
		++errorCount;
	if(i/j != 3)
		++errorCount;
}

int loopTest(){
	int i = 0;
	int j = 0;
	for(i = 10;i >=0; --i) j+=i;
	if(j!=55)
		++errorCount;
	return 0;
}

int paraTest(int n){
	int i;
	if(n == 0)
		return 0;
	i = paraTest(n-1);
	return i + n;
}

// test
int main(){
	int i;
	write(-1000);
	errorCount = 0;
	write(paraTest(10));
	eight_queen(0);
	write(gCount);
	structTest();
	breakContinueTest();
	for(i = 0; i < 25; ++i){
		write(callTest(i));
	}
	for(i = 0; i < 25; ++i){
		write(feb2(i));
	}
	logicArithmaticTest();
	arrayTest();
	scopeVariableTest();
	arithmaticTest();
	loopTest();
	relopTest();
	dfs(0,0);
	write(errorCount);
	write(-2000);
	return 0;
}
