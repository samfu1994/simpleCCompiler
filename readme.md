# <p align="center" >Compiler Principle Course Project</p>
# <p align="center" >Technique Report</p>

<br>

----------

<br>
# Introduction
C and some C-liked languages are the dominant programming languages. In this project,  a simplified compiler is designed and implemented, for the given programming language, namely SMALLC , which is a simplified C-liked language containing only the core part of C language. The compiler can translate SMALLC source codes to MIPS assembly codes. These assembly codes can run on the SPIM simulator, or can be assembled to machine code to run on a real computer. <br>
This project is roughly divided into two part, Syntax analyse and intermediate code generation is tightly coupled and finished simutaniously, and code generation is separated from the previous two. The three aspects is introduced below.
### syntax analyse
* Syntax analyse is done by the opensource library yacc. It will automatically generate a LALR parser for the given grammer.
* Error recorvery is not implemented in the project. So when the parser encountered a syntax error, it will terminate its process and report that error.

### intermediate code generation
* Three address code are chosen as intermediate code form for this project.
* During the parsing process,  all three address code is generated in only one scan of the source code. To achive this, backpatch technique is used in this project.
* The three address code hide all details of the target processor architecture from parser. Such as memory map, instruction set architecture, and so on.  Also it hide all details of the language from the back end of the compiler.
* Simple assamply-language-like operations is used in three-address code. See discussion below for details.
* During the three-address code generartion, the source code is also checked for semantical correctness.
* If a semantical error is found, the compiler will record it, and try to fix it using some simple rules rather than terminate the process.

### code generation
* Code generation is totaly saparated from intermediate generation. It will generate MIPS assembly language that implements the three addresas code generated during parsing.
* This part allocate and recycle regesters, manage memory, create run time enviroment for functions.
* Idealy,  no errors may occur in this phase. But the program may have bugs and make mistakes in three-address code generation. If any error occured in this phase, it is due to the errors occured in three address code generation.

### Other settings and defines in this project
*  Since all variables are integers in the end. All what the compiler can operate on is integers actually. So it is convenient to use word as a unit of size of all kinds of data. In code generation, all addresses will be shifted left by two bits, thus creating addresses for a word.
*  Boolean expressions and integer expressions can be converted if needed. Same as the defines in C language, a integer whose value is zero is logical false, otherwise true. And logical true is translated to integer 1, logical false is translated to integer 0.

----------
# implemenatation details
## intermediate code generation
### intermediate code define in this project
*  **Three address code structure:**<br>
In this project, three address code is stored as a structure containing four elements: operation code, two operands, and destination. Its defination is shown below:
	>Integer operstions<br>
	SEG 1: int operation<br>
	SEG 2:  string 	arg1<br>
	SEG 3:  string 	arg2(optional)<br>
	SEG 4:  string 	dst

	>JMP and BR operation<br>
	SEG 1:	int 	operation<br>
	SEG 2:	string  	arg1(optional)<br>
	SEG 3:  string 	arg2(optional)<br>
	SEG 4: string 	jmpAddress
*  **Three Address Code Operations** <br>

	all operations of three address code is listed below:

	<table border="0">
		<tr><th>Operation</th> <th>Type</th> <th>Description</th> <th>Remarks</th> </tr>
		<tr><td>OP\_ST</td> <td>data transfer</td> <td>store values in tempvar to memory</td> <td>the destination must be a variable, not a immediate number or temprary variable</td></tr>
		<tr><td>OP\_ADD</td> <td>integer</td> <td>arithmetic addition</td> <td></td></tr>
		<tr><td>OP\_SUB</td> <td>integer</td> <td>arithmetic substraction</td> <td></td></tr>
		<tr><td>OP\_MUL</td> <td>integer</td> <td>arithmetic multiplication</td> <td></td></tr>
		<tr><td>OP\_DIV</td> <td>integer</td> <td>arithmetic devision</td> <td></td></tr>
		<tr><td>OP\_MOD</td> <td>integer</td> <td>arithmetic modular</td> <td></td></tr>
		<tr><td>OP\_SHL</td> <td>integer</td> <td>arithmetic left shift</td> <td></td></tr>
		<tr><td>OP\_SHR</td> <td>integer</td> <td>arithmetic right shift</td> <td></td></tr>
		<tr><td>OP\_BAND</td> <td>integer</td> <td>logical bitwise and</td> <td></td></tr>
		<tr><td>OP\_BOR</td> <td>integer</td> <td>logical bitwise or</td> <td></td></tr>
		<tr><td>OP\_BNOT</td> <td>integer</td> <td>logical bitwise not</td> <td></td></tr>
		<tr><td>OP\_BXOR</td> <td>integer</td> <td>logical bitwise xor</td> <td></td></tr>
		<tr><td>OP\_PARA</td> <td>function</td> <td>set function call parameters</td> <td>all parameters must be transfered in order, must follow a call operation immediately</td></tr>
		<tr><td>OP\_BGE</td> <td>conditional brach</td> <td>jump when greater or equal</td> <td></td></tr>
		<tr><td>OP\_BGT</td> <td>conditional brach</td> <td>jump when greater then</td> <td></td></tr>
		<tr><td>OP\_BLT</td> <td>conditional brach</td> <td>jump when less then</td> <td></td></tr>
		<tr><td>OP\_BLE</td> <td>conditional brach</td> <td>jump when less or equal</td> <td></td></tr>
		<tr><td>OP\_BEQ</td> <td>conditional brach</td> <td>jump when equal</td> <td></td></tr>
		<tr><td>OP\_BNE</td> <td>conditional brach</td> <td>jump when not equal</td> <td></td></tr>
		<tr><td>OP\_JMP</td> <td>uncondition branch</td> <td>jump</td> <td></td></tr>
		<tr><td>OP\_CALL</td> <td>function</td> <td>call function</td> <td></td></tr>
		<tr><td>OP\_RET</td> <td>function</td> <td>return to caller</td> <td></td></tr> </table>
* **Three addresas code Operends** <br>
	* Since the three-addess code is **assembly-language-like** language without the structure of **highlevel language**, Some information is **missed** when transfer hige level language to three-address language, such as the action scope for a variable. To resolve this problem, I extended the three address code defination and added scope information into three address code to identify which scope the current line of code is running in.
	* Besides, all operands has there scope information too. This design is not necessary since the scope information can be found in symble table. It is must more convenient using this piece of information.
	* The variables in three-address has a format like **ID.SCOPE[:offset]**, the offset part if optional. In this way, all variables, no matter what type it is, can be visited using the address of that variable. This methid generated and simplified the variable modle in high-level language.
	* Temporary variables has format like **#indwexNumber**, "#" is the prefix for all temporary variables, This garentees that no overlape will occur between user defined variables and compiler generated temporary variables.
	* All immediate number is loaded into a tmevar before any operation, although it is not needed
* **Scope information in three-address-code**
	* There is a integer field in structure of three-address-code, indicateing the scope for current line of code. this is used for allocating and deallocating stack memory space for local variables. A better solution is to use new three-address-code instruction to indicate entering or leaving a specific scope, and change the pointer to symtable according to this instruction. There is no GOTO statement in SMALLC language, so it is impossiable that a jump instruction jumps out of current scope, space can be allocated dynamically. When entering a scope, allocate memory for variables defined in it, and deallocate when leaving this scope. A alternative method is allocating the maxmum memory a function can use at the begining, and deallocate when return. However, dynamically allocating memory may save much space need when there is recursive function calls.
* **Arrays**
	* In this project, multi dimension array is supported. The compiler support array defination of any number of dimensions.
	* Arrays are organized as row major in memory.
	* During parsing, the offset address of array is calculated and stored in a temporary variable. That is, parser will generate codes to calculate the offset address. So at the beck end point of view, there is no difference handling arrays with different number of dimension.
* **Structures**
	* In fact, Structure element visit can be reduced to one dimension array visit, using the element index in the structure as index of the memory. Again the idea of Struct in high-level language is hiden form the beck end of compiler.
	* A structure type variable can be assigned to another structure type variable, In this case, all elements in structure will be copied one by one. This work is also done by immediate code generator. The number of instructions generated is as mamy as the number of elements in the structure.
* **Bool-integer conversion**
	* Since the parser id bottom-up parser, I cannot determin whether the expression type is boolean or iinteger. So I can only choose one kind of output type. In this project, conditional jump will be generated for relation expression, and integer operation for other expressions. So there is problem when the expression is like:

			int i = (1 < 2);
	the branch operation has been generated and cannot be deleted, I used a new temporary variale and make the result equvalance to the following code:

			int i;
			if(1 < 2)
				i = 1;
			else
				i = 0;
	this method is complecated and not efficient. But I have to say that it is one of the drawbacks of one pass code generation.

	* Integer to bool conversion is much easier.Just add bequal branch instruction is enough.
* **FOR, CONTINUE and BREAK statement**
	* **for statement** is one of the most complicated statements. The execution order is very important.  But the code generated do not have the nature execution order of a for-loop, thus, a lot of jump operations is inserted into tree-address code again. This is another draw back of one-parse method, the order of instructions can not be controled, left a lot if jumps in genersated code.
	* **Break and Continue** is implemented using the same idea as IF statement. Using backpatch technique to add jump address after the address is avaliable. For-loop can be nested, so a global stack is need to record the nested structure of for-loop. Elements of this stack records the backpatch list of breaks and continues. Using this stack, it is also easy to determin whethwer a continue or break statement is in a for-loop or not.
* **lables in three-address-code**
	* For the convenience of debuging, all lables has a prefix of "@", it is not necessary at all actually. But it doesn't matter anyway. It is easier to distinguish temporary variables and lables generated by compiler now.
* ** Function calls:**
	* function calls are translated into two instructions, OP\_PARAM and OP\_JMP.
	* OP\_PARAM transfers parameters from caller to callee in order.
	* OP\_JMP jumps to the first instruction of callee.
	* If numbers of parameters transfered does not match the number of parameters needed, an error record will be generated. If there are more parameters then needed, only needed parameters will be actually transfered to garentee that parameter transfer will not overwrite other contant in stack. If the number of parameters is not enough, some parameters will be random value. The program may still run correctly even though some function call hava unmached parameter number.
* **Check missing return statement**
	* Return statement is essential both in three-address-code and final MIPS assembly code. In C and many other high-level programming languages, return statement is not a necessary part of a function. But both three-address-code and assembly codes do not have the concept of scope. If there is no return statement in a function to terminate its execution, it will continue to execute the next line of code and enters the next function, which is not acceptable.
	* Luckly, It is easy to use synthesis attributes to check whether a return statement exist in a function. If not, a warring information will be printed out and the three-address-code generator will add a `return 0;` statement automatically. Some compilers will return the last calculation result, maybe that is a better solution, the result is normally generated in the last step.
* **Check expression type**
	* There is only two types of expression in SMALLC, integer and boolean. The idea is try to convert boolean expression to integer or integer to boolean expression when needed.
		* In statement like for-loop and If, expression conversion are done using empty reduction right after the expression is reduced.
		* In calculations, the conversion is done after both two expression is reduced. Here, error array visit, structure element visit and other type error is checked and recorded.
* **Variable initial value**
	* There is two categries of variable in SMALLC, globle variables and local variables.
	* The address of globle variables is unique and is known during compiling process. They have only one instence, So can be initialized when load that program. As a result, Their initial value is stored in symble table, front end just leave that job to back end of this compiler.
	* The address of local variables is not unique, however. Only its offset in program stack is determined during compiling. Besides, thay can have multiple instence at the same time, a simple example is recursive call. So local variables can only be initialed just before it is used. This compiler will generate assign code in order to initiate local variables.
	* Because of the difference of strategies used to initiate variables, they have quite different restrictions. The initial value for a globle variables must be a constent, irrelevent to any other variables. For example the following globle variables declearation with initial value is not correct:

			int a = 0;
			int b = a;
	because the second initial value is another variable, thus not determined during compiling. Yon can say that it is definately determined because variable a is just been initialized and can only be 1. But from the compiler point of view it is not, it depend on the value of a, which break the law of compiler.<br>
	While the same code is correct for local variable declearation. Because local variable initial value is stored into that variables using additional instructions. In SMALLC, declearing local variables with initial value has the same effect as declear first and then assign its initial value.
* **Handle read and write function**
	* Read and write are two special functions that allow us to input data and read data out from condole provided by MIPS simulator.
	* The front end treat read and write as normal functions, at the initial stage of parsing, read and write are pushed into symble table directly, and call for read and write is also handled just like normal functions. This is not a good	way however, this problem is discussed further in code generartion.

### symble table organization
symtable is a key element in compiler. It is the bridge that connect front end and back end of the compiler. It is also essential for code generation because address for variables as well ad size of each statement block are recorded in symtable. Symtable has all missed information that three-address-code cannot give.

* There is a symtable table for each statement block, each symtable has a few element, including a map from variable name to a structure that record the variable information, function declearations, structure declearations. After the declearation part is parsed, the memory size needed for that scope is determined, and it is also stored in symtable.
* Symtables are stored in a vector. There is a field in each symtable that contains the index for its perent symtable. When a variable name occured, it should be either in current symtable or in its ancestor symtables. Otherwise this variable is not defined and it indicates a error in code.
* In fact, many information in symtable is not used. SMALLC language doesn't support nested function defination and nested structure defination, only nested statement blocks, So the variable action scope problem can be solved using renaming technique. That is, add a certain prefix or postfix related to scope to the variable name, then variables in different action scope will not overlape and user cannot visit a variable outof its action scope.

### Philosophy of names in program language
* Generally, in many program languages, there are two types of identifiers
	* identifiers that represent memeory address
	* identifiers that does not represent memeory address
* Identifiers such as **function names, variable names** are identifiers that represent a memeory address, they "exist" in the program physically. For example, a function name will represent an memeory address in machine language, its value is the address of the first instruction executed in that function. So I say it "exist" in that program. A more vivid example is globle variables. In my project implemenatation, globle variables is allocated in data segment and all has a lable, which is combination of common prefix and origin name.
* While name of structures, classes, and structure member names doesn't "exist" in that program. They are just description of special architecture. These names should never occur in the back end of compiler, since it is related to the language defination. And these names never represent a addresas or anything in machine code. The specific implemenatation for the same defination can vary because different compiler may have different underlying implemenatations and machenism for that language.
* in standared C language, the following declearation are legal:

		struct sname{
			int sname;
		};
		struct sname sname;
	There are four identical name "sname".
	* The first one is name for that structure. The second is name of an element in structure. The third name is also the name of structure, but this time it indicates that a variable of that architecture is decleared. These three names are virtule or abstruct names that does not related to an specific address or lable in machine code.
	* But the fourth "sname" is the name of variable, it generate a small block of memory, and may be visited.

	These names has very different attributes and should be stored saparatedly in symtable table. Among these names, only names that "exist" in machine code should be leaved in three-address-code, because they contain informations of the program. But othe names should not occur in three-address-code, for hiding information of source language.
	So there can be **many** identical names of "virtule" type in same scope, but **only one** "real" name in same scope.
* Another example is shown below:
	the following code is legal in standared C

		int main(){
			int main;
		}
	That is easy to understand because the two name "main" are in different scopes, there is no problem with this piece of code.
	But the following code is not correct:

		int main;
		int main(){
		}
	That is also easy to understand because both of the two names are "real" names, this kind of declearation make the name "main" mapped to two different address, one is function main, and another is integer variable main, the compiler can not decide which one to use when this name occur in an expression.
* In fact, the grammar of SMALLC garentees that normal variables and functions has different ways to visite. So the compiler actually can decide which declearation of the name "main" should be used. But this kind of declearation is still confusing.
* Also we can understand why in languages like C, C++ java, function names can be operands of an expression, can be stored in an array. You can store an instence of class or structure, but you can never store a class or structure, because they never really exist, they are just destination and restriction. Just like laws can never punish you, it is man who represent law who punishes you. Laws, and many such kind of idea never really exist in our lift, just like structures, classes never exist in program.

### comments on this part
* it is really necessary to use parse trees, doing things from top to bottom is much naturer and much easier. Using top-down method, all things is done recursively and elegant. there is more spece to optimize code and organize the order of code. Unlike bottom-up method, top-down method is much more flexiable and thus easier to implement.
* To get some kind of inherented attributes, I used several globle variables to transfer data, it is recommanded in documentation of bison but it is very likely that this kind of usage may introduce errors. The order of reduction is critical to the correctness of program. Again it is the drawback of bottom-up parsing.

## code generation
Code generation involves knowledge of MIPS architecture,  memory organization, and many other restrictions and standared. The code generated by this compiler should compatiable with code generated from other compilers. This means programmer of a compiler need to obey certain rules about how to use register, how to operate on hardware resources, and many other things. In one word, it is complicated, but obey these rules garentees a robuste system.
### Runtime enviroment
* **Memeory management**

	A mature compiler should devide memory for a function into two part, one is managed by caller, used for storing return address, parameters and such kind of data related to caller. The rest is managed by callee, used for storing local variables and other data used inside the function body. In my implemenatation, this idea is used implicitly by storing data into stack without moving stack pointer first. This is not safe if program run in a operating system and many process run simutaniously.

	The recommanded memory structure for MPIS is shown in this picture:
	![](memoryOrg.png )

	In this project, dynamic area is not considered because it is not supported in SMALLC language.
* **Saved registers:**

	$ra that records the return address of the process, $fp fram pointer of previous function, and any callee saved registers are saved before a function call and restored right after callee's return.
### Parameters:
	* Parameters are transfered from caller to callee only by storing data in the callee's stack, rather then using registers a0 to a3 to transfer the first few parameters.
	* There is a variables that records how many parameters has been stored in, and according to this variable, the address can be calculated. When a call is finally reached, the value of this variable is restored to its initial value.
	* When the the next three-address-code is "call read(var)", things are much different, this is discussed in read and write part in this section.

### Function Call
* Call is a special three-address-code instruction. The following work must be done:
	1. save live registers, callee may use these registers and change the value of these registers, it must be saved for future use.
	1. save special registers such ra.
	1. generate jal instruction
	1. restore saved registers and special registers
	1. get return value if needed.

	The order of instructions is vary important for call. Order is always important. Order is information, anyway. In computers, information is nothing but order of binary digits.
* From this part I get to understand the strength and drawback of divided processes into more functions. Functions are relatively separated from each other. Data is safe and cannot be overwriten or changed by any other functions, which creating a nature safe machenism. On the other hand, there is a lot of things to do in order to call a function, which will	influent the performance.

### Read and write
* The function write is defined as a standared function in SMALLC. So I just write this function in MIPS assembly language, and put it in the final code. Since this function obey the defination of function in SMALLC, it is easy	to do this.
* The function read violates the function defination in SMALLC, it passes integer parameters by reference rather then by value. But SMALLC does not have the pass-by-reference machenism at all. That means is has to be handled in a special way.
	* When translate parameters, code generater will read one instruction ahead to determin if the destination function is read, if so do nothing.
	* Code generator will fetch the previous instruction( it must be parameter), and put the address rather then value to the memory that used for transfer parameters.
	* Function read is also implemented by writing a few lines of MIPS assembly code and add it to the final code.
	* I think it is better if the defination of read can be changed. A read function should have no parameters and returns the value read from the terminal. This kind of defination do not violate the function defination of SMALLC and easier to understand. Making exceptions is always not good. A well defined language should generate its functionality and has trict restriction.
	* A better way to implement the read function is to change SMALLC grammar. By translating this special function call to a normal function call, or try to add feature of pass-parameters-by-reference, That is not that difficult.

### registers allocation
* In this project registers are allocated to a new temp variables when it first appeared in three-address-code. And deallocate when no following instruction need its value.
* To know when a temp variables is not needed anymore, I caned the code in a reverse order, and record the first occurence of each temp variables, this is the last occurence of a temp variables in execution order.
* I didn't handle the situation when there is not enough registers. It is hard to do this. Becides, there is 8 registers I used, t0 to t7, they are used up when the expression tree is a full binary tree and has hight of 8, that is the leves of this tree is as many as 256. I don't think a normal programmer will	write a expression that contains 256 variables. So i omited this part. If the registers is not enough, a error message will be recorded.

### instruction selection
* How to select instruction is not constructed in this project due to limited time.
* In fact the assembler can do a lot of such kind of work for us. For example, it will generate two instructions to load a large integer.

### Variables in code generation
* globle variables are defined in data segment using .word or .space, using variables name and a prefix as lable.
* local variables are allocated in stack dynamically. To store or fetch a word stored in local local variables, the code generator must first refer to symtable to get the offset of the variables and add this offset with frame pointer to get physic address, and then generate a instruction to fetch or store word.
* variables in a scope are not allocated until program entered that scope. This way reduced memory usage.

### Comments on this part
Due to time limitation and lack of programming skill, the only optimization I've implemented is calculate const expressions in expressions. The generated code contains many useless instructions such as load a variable when it is already loaded into other registers, or store variables several times, when only the last store is actually needed.

But this project indeed taught me a lot of things about what a processor will actually do when running a program writen in high-level language.
It is also a kind of adventure because I've tried different method to implements this compiler including building a parse tree.
Implement a simplified compiler is insteresting because you always have infinate ways to do one thing. Although there are certain rules and restrictions to follow, But there is always more freedom.
To understand the programming language itself is very insteresting.

# Testing
I've writen a test file and it is included in the project directory. In this test file the following features are tested:

* basic test including arithmetic expressions, logical expressions, function call, read, write, structure elements visit, structure assign to structure, array visit, Bool-integer conversion and break-continue statement.
* A normal recursive Fibonacci function.
* A recursive Fibonacci function that record intermediate result in a globle array to speed up calculation.
* An eight queue program
* Greatest common divisor
* A small function to test scope of variables
* A deep first search of a map

The test code is also compiled using a gcc compiler and executed on my laptop. The execution result of the program compiled using gcc compiler is the same as the output of the program compiled using my compiler.
But I didn't test most of error detection features. So the program may crash if input code involves errors.

<br>
<br>
<p align="right">Hao Fu <br> 12/19/2014 8:44:01 PM </p>