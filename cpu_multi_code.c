#include <stdlib.h>
#include <stdio.h>

#define FALSE 0
#define TRUE  1
//capacidade memoria em Bytes 
#define MAX 1024
//total de rgistradores
#define NUMREG 32
// registrador 31 $ra 
#define $ra reg[31]
// instrucao tipo R
#define R_TYPE 32
//macro para negar o valor do bit
#define not(b) ((b) == (1) ? (0) : (1) )
//macro para acessar as posicoes do opcode no reg de Instrucoes
#define IR26 26
#define IR27 27
#define IR28 28
#define IR29 29
#define IR30 30
#define IR31 31
//acessar apenas um bit
#define OneBit 1
//OS 19 Bits de seinal de controle
#define SIGNALCONTROL 19
#define CURRENTSTATE  4

int memoria[MAX];			// Memoria RAM
int reg[NUMREG];			// Banco de Registradores

char loop = 1; 				// variavel auxiliar que determina a parada da execucao deste programa
char returnSignal =	 0;	

/* ULA segue a especifica��o dada em sala de aula.
void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow)
args de entrada:		int a, int b, char ula_op
args de saida: 		int *result_ula, char *zero, char *overflow */
void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow)
{
	*overflow = 0;
	switch (ula_op)
		{
      case 0:    // and
				*result_ula = a & b;
				break;

      case 1:   // or
				*result_ula = a | b;
				break;

		case 2:  // add
			*result_ula = a + b;
			if ( (a >= 0 && b >= 0 && *result_ula < 0) || (a < 0 && b < 0 && *result_ula >= 0) )
				*overflow  = 1;
			break;

      case 6:   // sub
				*result_ula = a - b;
				if ( (a >= 0 && b < 0 && *result_ula < 0) || (a < 0 && b >= 0 && *result_ula >= 0) )
					*overflow  = 1;
				break;

      case 7:   // slt
				if(a < b)
					*result_ula = 1;
				else
					*result_ula = 0;
				break;
		}
		if (*result_ula == 0)
			*zero = 1;
		else
			*zero = 0;

		return;
}
/* takeNbits
*void TakeNbit(int var,int indice_bit, int n_bits ,int *result)
args de entrada:	int variavel_com_sinais, int indice_bit, int n_bits
args de saida: 		int *result  */
void takeNbits(int var,int indice_bit, int n_bits ,int *result){
	//Pega a quantidade bit apartir do indice informado
	int cmpbit = 0, aux=0, ii;
	int r=1;		
	if (n_bits >= 1 && n_bits < 32){
		//r = pow((double) 2, (double) n_bits) -1;			
		for (ii = 0; ii < n_bits; ii++)	r= r*2;
		cmpbit = r-1;
	}else {
		printf("Too many bits\n");
	}
	aux = var >> indice_bit;
	*result = aux & cmpbit;
}
/* signalsLogicEQ
*void signalsLogicEQ(int currentState,int *result)
args de entrada:	int currentState
args de saida: 		int *result  */
void signalsLogicEQ(int currentState,int *result){
	/*declarando os 19 sinais da Unidade de Controle*/
	int RegDst0		=0;		// 0	Lower-Bit
	int RegDst1   	=0;		// 1      
	int RegWrite	=0;		// 2
	int ALUSrcA		=0;		// 3

	int ALUSrcB0	=0;		// 4
	int ALUSrcB1	=0;		// 5
	int ALUOp0		=0;		// 6
	int ALUOp1		=0;		// 7

	int PCSrc0		=0;		// 8
	int PCSrc1		=0;		// 9
	int PCWriteCond	=0;		//10
	int PCWrite 	=0;		//11
	
	int IorD		=0;		//12
	int MemRead		=0;		//13
	int MemWrite 	=0;		//14
	int BNE 		=0;		//15
	
	int IRWrite		=0;		//16
	int MemtoReg0	=0;		//17
	int MemtoReg1	=0;		//18	High-Bit
	/*bits q representam o estado atual*/
	int S3=0,S2=0,S1=0,S0=0;
	//retira bit-a-bit que compoem o estado ATUAL 
	takeNbits(currentState, 0, 1, &S0);
	takeNbits(currentState, 1, 1, &S1);
	takeNbits(currentState, 2, 1, &S2);
	takeNbits(currentState, 3, 1, &S3);
	// estado 7
	/*equacoesLogicas p/Cada sinal de Controle*/
	RegDst0= (not(S3)& S2 & S1 & S0) 
	;
	//estado 12
	RegDst1= (S3 & S2 & not(S1) & not(S0))
	;
	// 4+7+11+12+14	
	RegWrite=  (( (((not(S3)&S2)&(not(S1)&not(S0)))|(((not(S3)&S2)&(S1&S0)))) | (((S3 & not(S2)) & (S1 & S0)) | ((S3&S2)&(not(S1)&not(S0))) ) )
	| ((S3&S2)&(S1&not(S0))))
	;
	// 2+6+8+10
	ALUSrcA= ((( (not(S3)&not(S2)) & (S1&not(S0)) ) | ((not(S3)&S2)&(S1&not(S0)))) | ( ((S3&not(S2)) & (not(S1)&not(S0)))
	| ((S3&not(S2)) & (S1&not(S0))) ))
	;		
	// 0+1
	ALUSrcB0= (((not(S3)&not(S2))&(not(S1)&not(S0))) | ((not(S3)&not(S2))&(not(S1)&S0)))
	;		
	// 1+2
	ALUSrcB1= (((not(S3)&not(S2))&(not(S1)&S0)) | ((not(S3)&not(S2))&(S1&not(S0))))
	;		
	// 6+8+10
	ALUOp0= ((((not(S3)&S2)&(S1&not(S0))) | ((S3&not(S2))&(not(S1)&not(S0))))
	| ((S3&not(S2))&(S1&not(S0))))
	;		
	// 6
	ALUOp1=0;
	//8+10+13+14
	PCSrc0= (((((S3&not(S2))&(not(S1)&not(S0)))|((S3&not(S2))&(S1&not(S0))))|((S3&S2)&(not(S1)&S0))) | ((S3&S2)&(S1&not(S0))))
	;		
	// 9+12+13+14
	PCSrc1= (((((S3 &not(S2))&(not(S1)&S0)) | ((S3 & S2) &(not(S1)&not(S0)))) | ((S3 & S2) &(not(S1)& S0))) | ((S3 & S2) & (S1 &not(S0))))
	;
	// 8+10
	PCWriteCond= (((S3&not(S2))&(not(S1)&not(S0))) | ((S3 &not(S2))& (S1 &not(S0))))
	;	
	// 0+9+12+13+14
	PCWrite= (((( ((not(S3)&not(S2))&(not(S1)&not(S0))) | ((S3 &not(S2))&(not(S1)& S0))) | ((S3 & S2) &(not(S1)&not(S0)))) |((S3 & S2) &(not(S1)& S0))) | ((S3 & S2) & (S1 &not(S0))))
	;		
	// 3+5
	IorD= (((not(S3))&(not(S2)& S1 & S0)) | ((not(S3)& S2) &(not(S1)& S0)))
	;			
	// estado 0+3
	MemRead= (((not(S3)&not(S2))&(not(S1)&not(S0))) | ((not(S3)&not(S2))&(S1 & S0))) 
	;
	// estado 5
	MemWrite= ((not(S3)& S2) &(not(S1)& S0));		
	// estado 10
	BNE= ((S3 &not(S2))& (S1 &not(S0)))
	;			
	//  estado 0
	IRWrite= ((not(S3)&not(S2))&(not(S1)&not(S0)))
	;		
	// estado 8
	MemtoReg0= ((not(S3)& S2) &(not(S1)&not(S0)));
	// estado 12
	MemtoReg1= ((S3 & S2) &(not(S1)&not(S0)));

	RegDst0 =  RegDst0 		<< 0;	
	RegDst1 =  RegDst1 		<< 1;	
	RegWrite=  RegWrite		<< 2;	
	ALUSrcA =  ALUSrcA 		<< 3;	

	ALUSrcB0 = ALUSrcB0		<< 4;		
	ALUSrcB1 = ALUSrcB1		<< 5;		
	ALUOp0   = ALUOp0  		<< 6;		
	ALUOp1   = ALUOp1  		<< 7;		

	PCSrc0     = PCSrc0     << 8;	
	PCSrc1     = PCSrc1     << 9;	
	PCWriteCond= PCWriteCond<< 10;
	PCWrite    = PCWrite    << 11;		
	
	IorD    = 	IorD    	<<12;			
	MemRead = 	MemRead 	<<13;		
	MemWrite= 	MemWrite	<<14;		
	BNE     = 	BNE     	<<15;			
	
	IRWrite   = IRWrite  <<16;		
	MemtoReg0 = MemtoReg0<<17;	
	MemtoReg1 = MemtoReg1<<18;	
	/*concatenando os resultados emuma variavel soah*/
	*result = MemtoReg0 | MemtoReg1 | IRWrite | BNE |
		MemWrite | MemRead | IorD | PCWrite | PCWriteCond | PCSrc1 | PCSrc0|
		ALUOp1 | ALUOp0 | ALUSrcB1 | ALUSrcB0 | ALUSrcA | RegWrite | RegDst1 | RegDst0;  
	//*result = *result & 0xFFFF;
}
/* funcaoProximoEstadoExplicita
*void funcaoProximoEstadoExplicita(int IR, int *controlSignal, int currentState, int *nextState)
args de entrada:	int IR, int *controlSignal, int currentState
args de saida: 		int *nextState  */
void funcaoProximoEstadoExplicita(int IR, int *controlSignal, int currentState, int *nextState){
	//variaveis para operar bit-a-bit na equacao logica 
	int OP1=0, OP0=0, OP2=0, OP3=0, OP4=0, OP5=0;
	// bits resultado p/cade EqLogica q compoem o proximo estado
	int N0,N1,N2,N3,N4,S3=0,S2=0,S1=0,S0=0;
	// variavel auxiliar para receber os sinais de controle do respectivo estado
	int sinal=0;
	/*Paser bit opcode to calcculate next state*/
	takeNbits(IR, IR26, OneBit, &OP0); //op0 = opcode[0] printf("op0: %d\n",op0);
	takeNbits(IR, IR27, OneBit, &OP1); //op1 = opcode[1] printf("op1: %d\n",op1);
	takeNbits(IR, IR28, OneBit, &OP2); //op2 = opcode[2] printf("op2: %d\n",op2);
	takeNbits(IR, IR29, OneBit, &OP3); //op3 = opcode[3] printf("op3: %d\n",op3);
	takeNbits(IR, IR30, OneBit, &OP4); //op4 = opcode[4] printf("op4: %d\n",op4);
	takeNbits(IR, IR31, OneBit, &OP5); //op0 = opcode[5] printf("op5: %d\n",op5);
	/*Paser bit Current State to calcculate next state*/
	takeNbits(currentState, 0, 1, &S0);//S0 = State[0]
	takeNbits(currentState, 1, 1, &S1);//S1 = State[1]
	takeNbits(currentState, 2, 1, &S2);//S2 = State[2]
	takeNbits(currentState, 3, 1, &S3);//S3 = State[3]
	/*EQUACOES LOGICAS PARA O CALCULO DO VALOR DE CADA BIT PARA O PROXIMO ESTADO!*/
	/*Funcao calcular os 19 sinais-de-controle do estado atual  */
	signalsLogicEQ( currentState, &sinal);
	//printf("%d\n",*controlSignal );
	// lower bit =(
	N0 = not(S0) * not(S1) & not(S2) & not(S3) | 
		not(S3) & not(S2) &  S1 & not(S0) & 
			( OP5 & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 | not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0) |
			OP5 & OP3 & OP1 & OP0 & not(OP4) & not(OP2) | 
			not(OP5) & not(OP4) & not(OP1) & not(OP2) & not(OP0) & OP3 | not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0) ) |
		not(S3) & not(S2) & not(S1) & S0 & ( not(OP5) & not(OP4) & not(OP3) & not(OP2) & not(OP0) & OP1 ) |
		not(S3) & S2 & S1 & not(S0)
	;
	N1 =(
		((not(S3) & not(S2)) & (not(S1) & S0)) & 
			(not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0) | not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 | not(OP5) & not(OP4) & OP3 & not(OP2) & not(OP1) & not(OP0) |
				not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0)| not(OP5) & not(OP4) & not(OP3) & not(OP2) & not(OP1) & not(OP0) |
				not(OP5) & not(OP4) & not(OP3) & OP2 & not(OP1) & OP0 | OP5 & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 |
				OP5 & not(OP4) & OP3 & not(OP2) & OP1 & OP0 ) | 
				not(S3) & not(S2) & S1 & not(S0) & ( not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 | 
				not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0) | 
				OP5 & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 | not(OP5) & not(OP4) & OP3 & not(OP2) & not(OP1) & not(OP0) ) |
			not(S3) & S2 & S1 & not(S0)
		)
	;
	N2 = (not(S3) & not(S2) & not(S1) & S0 & 
		( 	not(OP5) & not(OP4) & not(OP3) & not(OP2) & not(OP1) & not(OP0) |
			not(OP5) & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 ) | 
		not(S3) & not(S2) & S1 & not(S0) &
			(not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 | OP5 & not(OP4) & OP3 & not(OP2) & OP1 & OP0 |  not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0)) |
			not(S3) & not(S2) & S1 & S0 | not(S3) & S2 & S1 & not(S0)
		)
	;
	//High bit =O 
	N3 =( not(S3) & not(S2) & not(S1) & S0 & ( not(OP5) & not(OP4) & not(OP3) & not(OP2) & OP1 & not(OP0) |
			not(OP5) & not(OP4) & not(OP3) & OP2 & not(OP1) & OP0 | not(OP5) & not(OP4) & not(OP3) & OP2 & not(OP1) & not(OP0) | 
			not(OP5) & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 ) | 
		not(S3) & not(S2) & S1 & not(S0) & 
			( not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0) |
			not(OP5) & not(OP4) & OP3 & not(OP2) & not(OP1) & not(OP0) |
			not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0) |
			not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 )
		)
	;
	/*Concatenate bits to form next state*/
	N1 = N1 << 1;
	N2 = N2 << 2;
	N3 = N3 << 3;
	currentState = N0 | N1 | N2 | N3;
	currentState = currentState & 0xF;
	*nextState = currentState;
	*controlSignal = sinal;
}
/* UC principal
void UnidadeControle(int IR, int *sc);
args de entrada:		 int IR
args de saida: 		int *sc     */
void UnidadeControle(int IR, int *sinalControle){
	/*variaveisd auxiliares para concatenar sinal+estado*/
	int sinal=0,s,estadoAtual,e;
	int proximoEstado;
	// pega estado atual
	takeNbits(*sinalControle, SIGNALCONTROL, CURRENTSTATE, &estadoAtual);
	/*Funcao cm EQsLogicas implementadas*/
	funcaoProximoEstadoExplicita(IR, sinalControle , estadoAtual, &proximoEstado);
	takeNbits(*sinalControle,0,SIGNALCONTROL,&sinal);
	sinal= sinal & 0x7FFFF; // 0111 1111 1111 1111 1111 ->19sinais de controle
	e= proximoEstado; 
	// e= e ShiftLeft 19-bits
	e= proximoEstado << SIGNALCONTROL;
	//	Concatenate estado +sinal de controle
	sinal = e | sinal;
	*sinalControle= sinal;
}
/* Busca da Instrucao
void Busca_Instrucao(int sc, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew);
args de entrada:		int sc, int PC, int ALUOUT, int IR, int A, int B
args de saida: 		int *PCnew, int *IRnew, int *MDRnew     */
void Busca_Instrucao(int sinalControle, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew){
	int IRWrite=0, IorD=0;
	char zero, overflow;
	takeNbits(sinalControle, 16, 1, &IRWrite);
	takeNbits(sinalControle, 12, 1, &IorD);
	if(IRWrite == 1) {				 	//Se IRWrite == 1 'bit-16' Pelanza
		if(IorD == 0) {		 			//Se 	 IorD == 0 'bit-12', a memória é acessada na posição de PC
			*IRnew = memoria[PC];
		}
		else{							//Se IorD == 1, a memória é acessada na posição de ALUout
			*IRnew = memoria[ALUOUT];
		}
		*MDRnew = *IRnew << 16;			//extende endereco
		*MDRnew = *MDRnew >> 16;
		
		ula(PC, 0x4, 0x2, PCnew, &zero, &overflow);
	}
}
/* Decodifica Instrucao, Busca Registradores e Calcula Endereco para beq
void Decodifica_BuscaRegistrador(int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew);
args de entrada:		int sc, int IR, int PC, int A, int B,
args de saida: 		int *Anew, int *Bnew, int *ALUOUTnew     */
void Decodifica_BuscaRegistrador(int sinalControle, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew){
	int a,b, estado;
	char zero, overflow;
	takeNbits(sinalControle, 0, SIGNALCONTROL, &estado);
	if(estado == 0x30) {	//Se Estado1 (decode)
		if(IR == -1 || IR == 0) {
			*Anew = PC-4;
			ula(PC, 0, 0x02, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT = PC + DESVIO;
			loop = 0;
			returnSignal = 1;
			//return;
		}
		else {
			takeNbits(IR, 21, 5, &a); //*Anew recebe os 5 bits referentes ao registrador RS
			takeNbits(IR, 16, 5, &b); //*Bnew recebe os 5 bits referentes ao registrador RT
			*Anew = a;
			*Bnew = b;
			IR = IR << 16;
			IR = IR >> 14;
			ula(PC, IR, 0x02, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT = PC + DESVIO;
		}
	}
}
/*Executa TipoR, Calcula endereco para lw/sw e efetiva desvio condicional e incondicional
void Execucao_CalcEnd_Desvio(int sc, int A, int B, int IR, int PC, int ALUOUT, int *ALUOUTnew, int *PCnew);
args de entrada:		int sc, int A, int B, int IR, int PC, int ALUOUT
args de saida: 		 int *ALUOUTnew, int *PCnew */
void Execucao_CalcEnd_Desvio(int sinalControle, int A, int B, int IR, int PC, int ALUOUT, int *ALUOUTnew, int *PCnew){

	char zero, overflow, ulaop;
	int opcode, function, aluop=0;
	int adress, PCWriteCond, BNE, sinal;			/* Valor IMEDIATO 16-bits int adress = IR << 16;*/
	//endereco
	takeNbits(IR, 0, 16, &adress); 			// adress <= IR[0...15]
	//sinais
	takeNbits(sinalControle, 6, 2, &aluop); 			// 06- UALOp0 (ALUOp0) + 07- UALOp1 (ALUOp1) = 2 bits
	takeNbits(sinalControle, 10, 1, &PCWriteCond); 	// 10- PCEscCond (PCWriteCond) = 1 bit
	takeNbits(sinalControle,0,SIGNALCONTROL,&sinal);	// 0-18 => os 19 sinais
	ulaop = (char) aluop;

	if(returnSignal == 1) {
		return;
	}
	if(sinal == 0x48){
		takeNbits(IR, 0, 6, &function);		/*6 bits campo funcao*/
		switch(function) {
			case 0x20:	//add
				ula(reg[A], reg[B], 0x2, ALUOUTnew, &zero, &overflow);
				break;

			case 0x22:	//sub
				ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
				break;

			case 0x24:	//and
				ula(reg[A], reg[B], 0x0, ALUOUTnew, &zero, &overflow);
				break;

			case 0x25:	//or
				ula(reg[A], reg[B], 0x1, ALUOUTnew, &zero, &overflow);
				break;

			case 0x2a:	//slt
				ula(reg[A], reg[B], 0x7, ALUOUTnew, &zero, &overflow);
				break;
		}
	}
	/*Estado-2 Tipo-I 6 instrucoes LW/SW */
	else if(sinal == 0x28){
	//else if(sinalControle == 0x28) {	/*Pelanza-lw/sw*/
		//ula(reg[A], adress, 0x2, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT
		takeNbits(IR, 26, 6, &opcode);/*6 opcode*/
		switch(opcode) {
			case 0x08:	//addI
					//printf("ADDI IMEDIATO = %d ALUSrcA = %d \n", adress, A);
				ula(reg[A], adress, 0x02, ALUOUTnew, &zero, &overflow);
				break;

			case 0x0C:	//andi
					//printf("ANDI IMEDIATO = %d ALUSrcA = %d \n", adress, A );
				ula(reg[A], adress, 0x00, ALUOUTnew, &zero, &overflow);
				break;

			case 0x14: //jr Pelanza-JR jr rs
				*ALUOUTnew = PC;
				*PCnew = reg[A];
				break;

			case 0x15:	//jalr rs, rt => $ra = pc and goto rs;
				// passar reg
				//reg[B] = PC;
				//printf("*********Goto JALR = %d e REG: %d o PC= %d\n",reg[A]*4,B, PC);
				// RS = PC;
				*ALUOUTnew = PC;
				*PCnew = reg[A];
				break;

			case 0x23:	//LW
				//printf("LOADWORD A:%d Reg[A]: %d e Imediato: %d\n", A, reg[A], adress);
				ula(reg[A], adress, 0x02, ALUOUTnew, &zero, &overflow);
				break;

			case 0x2b:	//sw
			//	printf("STOREWORD A:%d Reg[A]: %d e Imediato: %d\n", A, reg[A], adress);
				ula(reg[A], adress, 0x02, ALUOUTnew, &zero, &overflow);
				break;
		}
	}   
	else if(sinal == 0x8548){        
	//else if (sinalControle == 0x8548){
		takeNbits(sinalControle, 15, 1, &BNE); 		// 15-(BNE) = 1 bit
		ula(reg[A], reg[B], 0x06, ALUOUTnew, &zero, &overflow);
		if (BNE){
			if(zero == FALSE) {
				*PCnew = ALUOUT;
			}
		}
	}
	else if(sinal == 0x548){
	//else if(sinalControle == 0x548) {	//BEQ
		ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
		
		if(zero == TRUE) {
			*PCnew = ALUOUT;
		}
	}
	else if(sinal == 0xA00){
	//else if(sinalControle == 0xA00) {			/*Pelanza-J*/
		adress = IR&0x3ffffff;	//realiza a extensão de sinal
		adress = adress << 2; 	// 30 bits
		*PCnew = adress;
	} 
	else if(sinal == 0x40A06){
	//else if(sinalControle == 0x40A06) {		/* Pelanza-JAL imediatte AluOp=00*/
		adress = IR&0x3ffffff;	//realiza a extensão de sinal
		adress = adress << 2; 	// 30 bits
		//printf("ADRESS calculado %d\n",adress );
		//printf("*********Goto JAL = %d e REG: 31 = (PC = %d)\n",adress, PC);
		*PCnew = adress;
	}	
}
/* Escreve no Bco de Regs resultado TiporR, Le memoria em lw e escreve na memoria em sw
void EscreveTipoR_AcessaMemoria(int sc, int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew);
args de entrada:	 int sc, int B, int IR, int ALUOUT, int PC
args de saida: 	 int *MDRnew, int *IRnew */
void EscreveTipoR_AcessaMemoria(int sinalControle, int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew){
	int RD,RT, opcode, sinal;
	if(returnSignal==1) {
		return;
	}
	takeNbits(sinalControle, 0, SIGNALCONTROL, &sinal);	// 0-18 => os 19 sinais
	if(sinal == 0x5){
		takeNbits(IR, 26, 6, &opcode);
		if(opcode == 0) {		//Tipo-R
			takeNbits(IR, 11, 5, &RD);
			reg[RD] = ALUOUT;
		}
		else{					//Tipo-I
			reg[B] = ALUOUT;
		}
	}
	else if(sinal == 0x30000){
		(*MDRnew) = (memoria[ALUOUT/4]) << 16;	//LW
		(*MDRnew) = (*MDRnew) >> 16;			//LW
	}
	else if(sinal == 0x5000){
		memoria[ALUOUT] = reg[B];				//SW
	}
}
/* Escreve no Bco de Regs o resultado da leitura da memoria feita por lw
void EscreveRefMem(sort int sc, int IR, int MDR, int ALUOUT);
args de entrada:		int sc, int IR, int MDR, int ALUOUT
args de saida: 		nao ha */
void EscreveRefMem(int sinalControle, int PC, int IR, int MDR, int ALUOUT){
	if(returnSignal == 1) {
		return;
	}
	int RT,RD, RegWrite=0, RegDest=00, MemtoReg=00;  		        
	takeNbits( sinalControle, 2, 1, &RegWrite); 		// 02-EscReg (RegWrite) = 1 bit
	//printf("RegWrite => %d\n", RegWrite);
	if(RegWrite) { 							/*estados 4,7,11,12,14 */  
		takeNbits( sinalControle, 0, 2, &RegDest);		/* 00-(RegDst0) : 01-(RegDst1) = 2 bits*/
		takeNbits( sinalControle,17, 2, &MemtoReg);		/* 00-(MemtoReg0) : 01-(MemtoReg1) = 2 bits*/
		switch (RegDest) {					//mux RegDest
			case 00:
				takeNbits( IR, 16, 5,&RT); 	// RT <= IR[16...20]; B
				
				//printf("00 RegDst: RT = %d MDR: %d\n",RT, MDR);
				//printf("MemtoReg = %d\n", MemtoReg);
				
				if (MemtoReg == 00) {		//RT = ALUOUT
					//printf("00 RegDst: RT = %d ALUOUT: %d\n",RT, ALUOUT);
					reg[RT] = ALUOUT;
				}else if (MemtoReg == 01){	//RT = MDR
					reg[RT] = MDR/4;
				}else if (MemtoReg == 02){	//RT = PC
					reg[RT] = PC;	
				}
			break;

			case 01:
				takeNbits( IR, 11, 5,&RD); 	// RD <= IR[11...15]; D
				//printf("01 RegDst RD = %d\n", RD);
				//reg[RD] = MDR/4;
				if (MemtoReg == 00) {		//RD = ALUOUT
					//printf("RD %d ALUOUT %d\n", RD, ALUOUT);
					reg[RD] = ALUOUT;
				}else if (MemtoReg == 01){	//RD = MDR
					reg[RD] = MDR/4;
				}else if (MemtoReg == 02){	//RD = PC
					reg[RD] = PC;	
				}
			break;
			
			case 02:
				//printf("10 Regdst $RA = %d\n",$ra);
				$ra = MDR/4;				   	//reg[31] = MDR; RA
				if (MemtoReg == 00) {		//RT = ALUOUT
					$ra = ALUOUT;
				}else if (MemtoReg == 01){	//RT = MDR
					$ra = MDR/4;
				}else if (MemtoReg == 02){	//RT = PC
					$ra = PC;	
				}
			
			break;
		}
	}
}

int main (int argc, char *argv[]){
	int PCnew = 0, IRnew, MDRnew, Anew, Bnew, ALUOUTnew;
	int PC = 0, IR=-1, MDR, A, B, ALUOUT;
	int sinalControle = 0;	   			// Sinais de Controle estado ZERO
					//                    cada bit determina um dos sinais de controle que saem da UC.
					// A posicao de cada sinal dentro do int esta especificada no enunciado
	char l,ula_op = 0;   			// Sinais de Controle de entrada para a ULA
					//                    sao usados apenas os 4 bits menos significativos dos 8 disponiveis.
	/*LENDO ARQUIVO DE ENTRADA (CONTEUDO da MEMORIA)*/
	int i=0,conteudo=0;
	while(scanf("%d",&conteudo) !=EOF){
		memoria[i] = conteudo;
		i= i+4;
	}

	while(loop){
		// aqui comeca um novo ciclo
		// abaixo estao as unidades funcionais que executarao em todos os ciclos
		// os sinais de controle em sinalControle impedirao/permitirao que a execucao seja, de fato, efetivada
		UnidadeControle(IR, &sinalControle);
		Busca_Instrucao(sinalControle, PC, ALUOUT, IR, A, B, &PCnew, &IRnew, &MDRnew);
		Decodifica_BuscaRegistrador(sinalControle, IR, PC, A, B, &Anew, &Bnew, &ALUOUTnew);
		Execucao_CalcEnd_Desvio(sinalControle, A, B, IR, PC, ALUOUT, &ALUOUTnew, &PCnew);
		EscreveTipoR_AcessaMemoria(sinalControle, B, IR, ALUOUT, PC, &MDRnew, &IRnew);
		EscreveRefMem(sinalControle, PC, IR, MDR, ALUOUT);
		// contador que determina quantos ciclos foram executados
		// atualizando os registradores temporarios necessarios ao proximo ciclo.
		PC	 	= PCnew;
		IR		= IRnew;
		MDR		= (MDRnew);
		A		= Anew;
		B		= Bnew;
		ALUOUT 	= ALUOUTnew;
	} // fim do while(loop)

	// impressao da memoria para verificar se a implementacao esta correta

		int ii;
		printf("TODOS OS SINAIS EM DECIMAL\n");
		printf("PC=%d   \tIR=%d   \tMDR=%d\n",PC,IR,MDR);
		printf("A=%d    \tB=%d      \tALUOUT=%d\n",A,B,ALUOUT);
		printf("Controle= %d\n",sinalControle);

		printf("\nBANCO REGISTRADORES\n");
		for ( ii = 0; ii < 8; ii++)
		{
			printf("R%d=%d      \t", ii+0,reg[ii]);
			printf("R%d=%d       \t", ii+8,reg[ii+8]);
			printf("R%d=%d       \t", ii+16,reg[ii+16]);
			printf("R%d=%d ", ii+24,reg[ii+24]);
			printf("\n");
		}
		printf("\n\nMEMORIA\n");
		for (ii = 0; ii < 31 ; ii+=4) {
			printf("Memoria[%d]=%d       \t", ii,    memoria[ii]);
			printf("Memoria[%d]=%d      \t", ii+32, memoria[ii+32]);
			printf("Memoria[%d]=%d       \t", ii+64, memoria[ii+64]);
			printf("Memoria[%d]=%d     \t", ii+96, memoria[ii+96]);
			printf("\n");
		}
	exit(0);
} 

