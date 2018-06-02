
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FALSE 0
#define TRUE  1
#define MAX 1024
#define NUMREG 32
#define $ra reg[31]

#define R_TYPE 32

#define not(b) ((b) == (1) ? (0) : (1) )
#define IR26 26
#define IR27 27
#define IR28 28
#define IR29 29
#define IR30 30
#define IR31 31

#define OneBit 1

int memoria[MAX];		// Memoria RAM
int reg[NUMREG];		// Banco de Registradores

char loop = 1; 		// variavel auxiliar que determina a parada da execucao deste programa
char returnSignal = 0;

//variavel pra testes
char pausa;


void takeNbits(int var,int idx_start_bit, int n_bits ,int *result){
/******I've weed + I've Cooffe = I've Great IDEIA!"*/
	int cmpbit = 0, aux=0;
	double r=0.00;		
	if (n_bits >= 1 && n_bits < 32){
		r = pow((double) 2, (double) n_bits) -1;			
		cmpbit = (int)r;
	}else {
		printf("quantidade eerrada de bits\n");
	}
	aux = var >> idx_start_bit;
	*result = aux & cmpbit;
}

void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow)
{
	printf("ULA A=%d  B=%d OP=%d\n", a, b, ula_op);
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


void funcaoProximoEstadoExplicita(int IR, int *controlSignal, int currentState, int *nextState){
	int OP1=0, OP0=0, OP2=0, OP3=0, OP4=0, OP5=0;
	int N0,N1,N2,N3,N4,S3,S2,S1,S0;
	int  var, v;
	// variavel auxiliar recebe estado atual
	var = *nextState;

	takeNbits(IR, IR26, OneBit, &OP0); //op0 = opcode[0] printf("op0: %d\n",op0);
	takeNbits(IR, IR27, OneBit, &OP1); //op1 = opcode[1] printf("op1: %d\n",op1);
	takeNbits(IR, IR28, OneBit, &OP2); //op2 = opcode[2] printf("op2: %d\n",op2);
	takeNbits(IR, IR29, OneBit, &OP3); //op3 = opcode[3] printf("op3: %d\n",op3);
	takeNbits(IR, IR30, OneBit, &OP4); //op4 = opcode[4] printf("op4: %d\n",op4);
	takeNbits(IR, IR31, OneBit, &OP5); //op0 = opcode[5] printf("op5: %d\n",op5);

	takeNbits(currentState, 0, 1, &S0);
	takeNbits(currentState, 1, 1, &S1);
	takeNbits(currentState, 2, 1, &S2);
	takeNbits(currentState, 3, 1, &S3);
	//next
	takeNbits(currentState, 0, 1, &N0);
	takeNbits(currentState, 1, 1, &N1);
	takeNbits(currentState, 2, 1, &N2);
	takeNbits(currentState, 3, 1, &N3);
	var= OP5<<5 | OP4<<4 | OP3<<3 |OP2<<2 | OP1<<1 | OP0<<0 ;
	N1 = N1 << 1;
	N2 = N2 << 2;
	N3 = N3 << 3;
	v = N0 | N1 | N2 | N3;
	v = currentState & 0x0F;
	printf("-----------------------------------------------------------------ESTADO: %d\n",v );
	printf("ESTADO binario: %d %d %d %d ",S3, S2, S1, S0 );
	printf("OPCODE: %d %d %d %d %d %d =>",OP5,OP4, OP3, OP2, OP1, OP0 );
	printf(" Decimal: %d \n",var);

	switch (currentState) {
		case 0 :
			*controlSignal = 0x12810;//FETCH
			var = 0x12810;//FETCH
		break;
		case 1 :
			*controlSignal = 0x30;	//DECODE
			//var = 
		break;
		case 2 :
			*controlSignal = 0x28;//TIPO-I
		break;
		case 3 :
			*controlSignal = 0x3000;//LW
		break;
		case 4 :
			*controlSignal = 0x20004;//WB-LW
		break;
		case 5 :
			*controlSignal = 0x5000;//SW
		break;
		case 6 :
			*controlSignal = 0x48;	//R
		break;
		case 7 :
			*controlSignal = 0x05;	//R-COMPLETE
		break;
		case 8 :
			*controlSignal = 0x548;	//BEQ
		break;
		case 9 :
			*controlSignal = 0xA00;	//JUMP
		break;
		case 10 :
			*controlSignal = 0x8548;//BNE
		break;
		case 11 :
			*controlSignal = 0x04;	//ADDI // ANDI
		break;
		case 12 :
			*controlSignal = 0x40206;//JAL
		break;
		case 13 :	
			*controlSignal = 0xB00;	//JR
		break;
		case 14 :
			*controlSignal = 0x0B04; //JALR
		break;
	}
	/*EQUACOES LOGICAS PARA O CALCULO DO VALOR DE CADA BIT PARA O PROXIMO ESTADO!*/
	// lower bit
	N0 = not(S0) * not(S1) & not(S2) & not(S3) | 
		not(S3) & not(S2) &  S1 & not(S0) & 
			( OP5 & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 | not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0) |
			OP5 & OP3 & OP1 & OP0 & not(OP4) & not(OP2) | 
			not(OP5) & not(OP4) & not(OP1) & not(OP2) & not(OP0) & OP3 | not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0) ) |
		not(S3) & not(S2) & not(S1) & S0 & ( not(OP5) & not(OP4) & not(OP3) & not(OP2) & not(OP0) & OP1 ) |
		not(S3) & S2 & S1 & not(S0)
	;
	
	N1 = not(S3) & not(S2) & not(S1) & S0 & 
			(not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0) | not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 | not(OP5) & not(OP4) & OP3 & not(OP2) & not(OP1) & not(OP0) |
				not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0)| not(OP5) & not(OP4) & not(OP3) & not(OP2) & not(OP1) & not(OP0) |
				not(OP5) & not(OP4) & not(OP3) & OP2 & not(OP1) & OP0 | OP5 & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 |
				OP5 & not(OP4) & OP3 & not(OP2) & OP1 & OP0 ) | 
				not(S3) & not(S2) & S1 & not(S0) & ( not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 | 
				not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0) | 
				OP5 & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 | not(OP5) & not(OP4) & OP3 & not(OP2) & not(OP1) & not(OP0) ) |
			not(S3) & S2 & S1 & not(S0)
	;

	N2 = not(S3) & not(S2) & not(S1) & S0 & 
		( 	not(OP5) & not(OP4) & not(OP3) & not(OP2) & not(OP1) & not(OP0) |
			not(OP5) & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 ) | 
		not(S3) & not(S2) & S1 & not(S0) &
			(not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 | OP5 & not(OP4) & OP3 & not(OP2) & OP1 & OP0 |  not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0)) |
			not(S3) & not(S2) & S1 & S0 | not(S3) & S2 & S1 & not(S0)
	;
	//High bit
	N3 = not(S3) & not(S2) & not(S1) & S0 & ( not(OP5) & not(OP4) & not(OP3) & not(OP2) & OP1 & not(OP0) |
			not(OP5) & not(OP4) & not(OP3) & OP2 & not(OP1) & OP0 | not(OP5) & not(OP4) & not(OP3) & OP2 & not(OP1) & not(OP0) | 
			not(OP5) & not(OP4) & not(OP3) & not(OP2) & OP1 & OP0 ) | 
		not(S3) & not(S2) & S1 & not(S0) & 
			( not(OP5) & not(OP4) & OP3 & OP2 & not(OP1) & not(OP0) |
			not(OP5) & not(OP4) & OP3 & not(OP2) & not(OP1) & not(OP0) |
			not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & not(OP0) |
			not(OP5) & OP4 & not(OP3) & OP2 & not(OP1) & OP0 )
	;

	var = *controlSignal;
	printf("SINAL CONTROLE : %d\n", var);
	
	N1 = N1 << 1;
	N2 = N2 << 2;
	N3 = N3 << 3;
	currentState = N0 | N1 | N2 | N3;
	currentState = currentState & 0xF;
	*nextState = currentState;
	printf("\n");
	//funcaoProximoEstadoExplicita(0x8c480000, currentState, controlSignal, nextState, c);
	
}

void UnidadeControle(int IR, int *sc, int state, int *nextState){
	//comeca com -1 = 111111
	int opcode; 
	takeNbits(IR, 26, 6, &opcode); //opcode = IR[31-26]
	int currentState;
	//void funcaoProximoEstadoExplicita(int opcode, int *controlSignal, int currentState, int *nextState)
	funcaoProximoEstadoExplicita(IR, sc , state, nextState);

}

void Busca_Instrucao(int sc, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew){
	int IRWrite=0, IorD=0;
	char zero, overflow;
	takeNbits(sc, 16, 1, &IRWrite);
	takeNbits(sc, 12, 1, &IorD);
	//printf("IRWrite =>> %d\n",IRWrite);
	//printf("IorD    =>> %d\n",IorD);
	if(IRWrite == 1) {				 //Se IRWrite == 1 'bit-16' Pelanza
		if(IorD == 0) {		 		//Se 	 IorD == 0 'bit-12', a memória é acessada na posição de PC
			//printf("Pccccpc %d\n", PC);
			//printf("ALUOUT %d\n", ALUOUT/4);
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

void Decodifica_BuscaRegistrador(int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew){
	int a,b;
	char zero, overflow;
	if(sc==0x30) {	//Se Estado1 (decode)
		printf("DECODEEE\n");
		if(IR == -1) {
			loop = 0;
			printf("HALT!\n");
			returnSignal = 1;
			return;
		}
		else {
			takeNbits(IR, 21, 5, &a); //*Anew recebe os 5 bits referentes ao registrador RS
			takeNbits(IR, 16, 5, &b); //*Bnew recebe os 5 bits referentes ao registrador RT
			*Anew = a;
			*Bnew = b;
			IR = IR << 16;
			IR = IR >> 14;
			//06- UALOp0 (ALUOp0) 
			//07- UALOp1 (ALUOp1) 
			ula(PC, IR, 0x02, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT = PC + DESVIO;
		}
	}
}

void Execucao_CalcEnd_Desvio(int sc, int A, int B, int IR, int PC, int ALUOUT, 
														int *ALUOUTnew, int *PCnew){

	char zero, overflow, ulaop;
	int op, function, aluop=0;
	int adress, PCWriteCond, BNE;			/* Valor IMEDIATO 16-bits int adress = IR << 16;*/
	//endereco
	takeNbits(IR, 0, 16, &adress); 			// adress <= IR[0...15]
	//sinais
	takeNbits(sc, 6, 2, &aluop); 			// 06- UALOp0 (ALUOp0) + 07- UALOp1 (ALUOp1) = 2 bits
	takeNbits(sc, 10, 1, &PCWriteCond); 	// 10- PCEscCond (PCWriteCond) = 1 bit
	
	ulaop = (char) aluop;

	if(returnSignal == 1) {
		return;
	}
	if(sc == 0x48){
		takeNbits(IR, 0, 6, &function);		/*6 bits campo funcao*/
		switch(function) {
			case 0x20:	//add
				printf("A=== %d B=== %d\n",A, B );
				printf("REG[A]=== %d REG[B]=== %d\n",reg[A], reg[B]);
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
	else if(sc == 0x28) {	/*Pelanza-lw/sw*/
		//ula(reg[A], adress, 0x2, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT
		takeNbits(IR, 26, 6, &function);		/*6 opcode*/

		switch(function) {
			case 0x08:	//addI
					printf("ADDI IMEDIATO = %d ALUSrcA = %d \n", adress, A);
				ula(reg[A], adress, 0x02, ALUOUTnew, &zero, &overflow);
				break;

			case 0x0C:	//andi
					printf("ANDI IMEDIATO = %d ALUSrcA = %d \n", adress, A );
				ula(reg[A], adress, 0x00, ALUOUTnew, &zero, &overflow);
				break;

			case 0x14: //jr Pelanza-JR jr rs
				*PCnew = reg[A];
				break;

			case 0x15:	//jalr rs, rt => $ra = pc and goto rs;
				// passar reg
				//reg[B] = PC;
				printf("*********Goto JALR = %d e REG: %d o PC= %d\n",reg[A]*4,B, PC);
				// RS = PC;
				*ALUOUTnew = PC;
				*PCnew = reg[A];
				break;

			case 0x23:	//LW
				printf("LOADWORD A:%d Reg[A]: %d e Imediato: %d\n", A, reg[A], adress);
				ula(reg[A], adress, 0x02, ALUOUTnew, &zero, &overflow);
				break;

			case 0x2b:	//sw
				printf("STOREWORD A:%d Reg[A]: %d e Imediato: %d\n", A, reg[A], adress);
				ula(reg[A], adress, 0x02, ALUOUTnew, &zero, &overflow);
				break;
		}
	}           
	else if (sc == 0x8548){
		takeNbits(sc, 15, 1, &BNE); 		// 15-(BNE) = 1 bit
		ula(reg[A], reg[B], 0x06, ALUOUTnew, &zero, &overflow);
		if (BNE){
			if(zero == FALSE) {
				*PCnew = ALUOUT;
			}
		}
	}
	else if(sc == 0x548) {	//BEQ
		ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
		
		if(zero == TRUE) {
			*PCnew = ALUOUT;
		}
	}
	else if(sc == 0xA00) {			/*Pelanza-J*/
		adress = IR&0x3ffffff;	//realiza a extensão de sinal
		adress = adress << 2; 	// 30 bits
		*PCnew = adress;
	} 
	else if(sc == 0x40206) {		/* Pelanza-JAL imediatte AluOp=00*/
		adress = IR&0x3ffffff;	//realiza a extensão de sinal
		adress = adress << 2; 	// 30 bits
		printf("ADRESS calculado %d\n",adress );
		printf("*********Goto JAL = %d e REG: 31 = (PC = %d)\n",adress, PC);
		*PCnew = adress;
	}	
}

void EscreveTipoR_AcessaMemoria(int sc, int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew){
	int RD,RT, opcode;
	if(returnSignal==1) {
		return;
	}
	if(sc == 0x5) {	//estado 7
		takeNbits(IR, 26, 6, &opcode);
		//if(IR >> 27 == 0) {	Tipo-R
		if(opcode == 0) {		//Tipo-R
			takeNbits(IR, 11, 5, &RD);
			reg[RD] = ALUOUT;
		}
		else{	//Tipo-I
			//reg[B] = ALUOUT;
			reg[B] = ALUOUT;
		}
		printf("\t\tresultado da ula: %d\n", ALUOUT);
	}
	else if(sc == 0x30000) {	//se LW
		(*MDRnew) = (memoria[ALUOUT/4]) << 16;	//LW
		(*MDRnew) = (*MDRnew) >> 16;	//LW
		printf("----------ALUOUT: %d\n", ALUOUT);
		printf("MDRnew: %d\n", *MDRnew);
	}
	else if(sc == 0x5000) {	        //estado 5 se SW
		memoria[ALUOUT] = reg[B];	//SW
	}
}

void EscreveRefMem(int sc, int PC, int IR, int MDR, int ALUOUT){
	if(returnSignal == 1) {
		return;
	}
	int RT,RD, RegWrite=0, RegDest=00, MemtoReg=00;  		        
	takeNbits( sc, 2, 1, &RegWrite); 		// 02-EscReg (RegWrite) = 1 bit
	printf("RegWrite => %d\n", RegWrite);
	if(RegWrite) { 							/*estados 4,7,11,12,14 */  
		takeNbits( sc, 0, 2, &RegDest);		/* 00-(RegDst0) : 01-(RegDst1) = 2 bits*/
		takeNbits( sc,17, 2, &MemtoReg);		/* 00-(MemtoReg0) : 01-(MemtoReg1) = 2 bits*/
		switch (RegDest) {					//mux RegDest
			case 00:
				takeNbits( IR, 16, 5,&RT); 	// RT <= IR[16...20]; B
				
				printf("00 RegDst: RT = %d MDR: %d\n",RT, MDR);
				printf("MemtoReg = %d\n", MemtoReg);
				
				if (MemtoReg == 00) {		//RT = ALUOUT
					printf("00 RegDst: RT = %d ALUOUT: %d\n",RT, ALUOUT);
					reg[RT] = ALUOUT;
				}else if (MemtoReg == 01){	//RT = MDR
					reg[RT] = MDR/4;
				}else if (MemtoReg == 02){	//RT = PC
					reg[RT] = PC;	
				}
			break;

			case 01:
				takeNbits( IR, 11, 5,&RD); 	// RD <= IR[11...15]; D
				printf("01 RegDst RD = %d\n", RD);
				//reg[RD] = MDR/4;
				if (MemtoReg == 00) {		//RD = ALUOUT
					printf("RD %d ALUOUT %d\n", RD, ALUOUT);
					reg[RD] = ALUOUT;
				}else if (MemtoReg == 01){	//RD = MDR
					reg[RD] = MDR/4;
				}else if (MemtoReg == 02){	//RD = PC
					reg[RD] = PC;	
				}
			break;
			
			case 02:
				printf("10 Regdst $RA = %d\n",$ra);
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


int main (int argc, char *argv[])
{
	int PCnew = 0, IRnew, MDRnew, Anew, Bnew, ALUOUTnew;
					// Registradores auxiliares usados para a escrita dentro de um ciclo.
					// Guardam temporariamente o resultado durante um ciclo. Os resultados aqui armazenados estarao
					// disponiveis para leitura no final do ciclo atual (para que o mesmo esteja disponivel apenas no
					// incio do ciclo seguinte).
					// Em um ciclo sempre e lido o conteudo de um registrador atualizado no ciclo anterior.

	int PC = 0, IR=-1, MDR, A, B, ALUOUT;
					// Registradores especiais usados para a leitura em um ciclo.
					// Guardam os resultados que poderao ser utilizados ja neste ciclo, pois foram atualizados no final
					// do ciclo anterior.
					// Ex.: em um ciclo, o resultado da ULA e inserido inicialmente em ALUOUTnew.  Apenas no final
					// do ciclo esse conteudo podera ser atribuido para ALUOUT, para que o mesmo possa ser
					// usado no ciclo seguinte.
					// Em um ciclo sempre e lido o conteudo de um registrador atualizado no ciclo anterior.

	int sc = 0;	   	// Sinais de Controle estado ZERO
					//                    cada bit determina um dos sinais de controle que saem da UC.
					// A posicao de cada sinal dentro do int esta especificada no enunciado
	char l,ula_op = 0;   			// Sinais de Controle de entrada para a ULA
					//                    sao usados apenas os 4 bits menos significativos dos 8 disponiveis.

	int nr_ciclos = 0; 			// contador do numero de ciclos executados

/*
As variaveis zero e overflow nao precisam definidas na main.
Serao argumentos de retorno da ula e devem ser definidas localmente nas
rotinas adequadas.
char  zero, overflow;  			// Sinais de Controle de saida da UL: bit zero e bit para indicar overflow
*/
	
	memoria[0]= 201326610 ;  //  	# jal main:
	memoria[4]= 537395210 ;  //  	#executa: 	addi $t0, $zero, 10 	#04-01 nr
	memoria[8]= 537460737 ;  // 	addi $t1, $zero, 1
	memoria[12]= 537526274 ;  // 	addi $t2, $zero, 2  	#12-03 aux para nr 2
	memoria[16]= 277348355 ;  // loop_exec1:beq $a0, $t0, sai_loop	#16-04 enquanto $a0 < $t0 => farah 10 itera desloca 3 palavras à frente)
	memoria[20]= 8986656   ;  //  	add $a0, $a0, $t1 
	memoria[24]= 13185058  ;  // 	sub $a2, $a2, $t1
	memoria[28]= 134217732 ;  //  	j loop_exec1
	memoria[32]= 8929322   ;  // 	sai_loop:	slt $t0, $a0, $t0 		#32-08 setando $t0 para zero  (como $a0==10 e $t0==10, entao $t0 = 0)
	memoria[36]= 549912577 ;  //  	addi $a3, $a2, 1		#36-09 $a3 = 3
	memoria[40]= 14702629  ; //		or 	$t3, $a3, $zero		#40-10 $t3 = 3
	memoria[44]= 552337409 ; // 	addi $t4, $a3, 1		#44-11 $t4 = 4
	memoria[48]= 25518116  ; //		and  $t4, $t4, $a1  	#48-12 $t4 = 4
	memoria[52]= 361496579 ; // 	bne $t4, $t4, naopula	#52-13 nao efetiva o bne, pois $t4 == $t4  :-)
	memoria[56]= 538443876 ; // 	addi $t8, $zero, 100	#56-14 carrega endereco para salvar na memoria
	memoria[60]= 2936668160; //  	sw $t2, 0($t8) 			#60-15 salva $t2 (tem vlr 2) na memoria
	memoria[64]= 2400321536; //  	lw $s2, 0($t8) 			#64-16 recupera vlr 2 para s2
	memoria[68]= 1407188992; //  	naopula: jr $ra    		#68-17 fim de executa. Volta para a main.
	memoria[72]= 813957120 ; // 	main: andi $a0, $a0, 0	#72-18 $a0 tem todos os bits como 0
	memoria[76]= 537264127 ; // 	addi $a1, $zero, -1  	#76-19 $a1 tem todos os bits como 1
	memoria[80]= 816185356 ; //		andi $a2, $a1, 12		#80-20 $a2 tem 12 para ser decrementado em executa
	memoria[84]= 538443780 ; // 	addi $t8, $zero, 4		#84-21 recupera endereco de executa (#04)
	memoria[88]= 1461649408; //		jalr $t8,$ra			#88-22 desvia fluxo para funcao executa
	memoria[92]= 4294967295; // 	###### FIM da execução
	memoria[96] = -1;
	/*
	memoria[0]= 538443876 ; // 	addi $t8, $zero, 100	#56-14 carrega endereco para salvar na memoria
	memoria[0]= 1407188992; //  	naopula: jr $ra    		#68-17 fim de executa. Volta para a main.
	memoria[4]= 2936668160; //  	sw $t2, 0($t8) 			#60-15 salva $t2 (tem vlr 2) na memoria
	memoria[8]= 2400321536; //  	lw $s2, 0($t8) 			#64-16 recupera vlr 2 para s2
	memoria[12] = 0xac480000;  // 1010 1100 0100 1000 0000 0000 0000 0000  sw $t0, 0($v0)	
	memoria[72]= 813957120 ; // 	main: andi $a0, $a0, 0	#72-18 $a0 tem todos os bits como 0
	*/
	
	reg[4]  = 0;   // $a0
	reg[5]  = -1;  // $a1
	reg[6]  = 0;   // $a2
	reg[31] = 72;  // $ra
	

	int state =0, nextState=0;  
	
	while(loop){
		// aqui comeca um novo ciclo
		// abaixo estao as unidades funcionais que executarao em todos os ciclos
		// os sinais de controle em sc impedirao/permitirao que a execucao seja, de fato, efetivada
		UnidadeControle(IR, &sc,state, &nextState);
		Busca_Instrucao(sc, PC, ALUOUT, IR, A, B, &PCnew, &IRnew, &MDRnew);
		Decodifica_BuscaRegistrador(sc, IR, PC, A, B, &Anew, &Bnew, &ALUOUTnew);
		Execucao_CalcEnd_Desvio(sc, A, B, IR, PC, ALUOUT, &ALUOUTnew, &PCnew);
		EscreveTipoR_AcessaMemoria(sc, B, IR, ALUOUT, PC, &MDRnew, &IRnew);
		EscreveRefMem(sc, PC, IR, MDR, ALUOUT);
		// contador que determina quantos ciclos foram executados
		nr_ciclos++;
		// atualizando os registradores temporarios necessarios ao proximo ciclo.
		
		printf("PC=%d   IR=%d   MDR=%d\n",PC,IR,MDR);
		printf("A=%d   B=%d   ALUOUT=%d\n",A,B,ALUOUT);
		printf("CONTROLE=%d\n",sc);

		PC	 	= PCnew;
		IR		= IRnew;
		MDR		= (MDRnew*4);
		A		= Anew;
		B		= Bnew;
		ALUOUT 	= ALUOUTnew;
		state   = nextState;

		printf("NESXT============================================================\n");
		scanf("%c",&l);
		// aqui termina um ciclo
	} // fim do while(loop)

	// impressao da memoria para verificar se a implementacao esta correta

		int ii;
		printf("PC=%d   IR=%d   MDR=%d\n",PC,IR,MDR);
		printf("A=%d   B=%d   ALUOUT=%d\n",A,B,ALUOUT);
		printf("CONTROLE=%d\n",sc);
		printf("BANCO REGISTRADORES\n");
		for ( ii = 0; ii < NUMREG; ii++)
		{
			printf("R%d=%d\t", ii,reg[ii]);
			if ((ii % 8) == 0)printf("\n");
		}
		printf("\n\nMEMORIA\\n");
		for (ii = 0; ii < 124 ; ii+=4) {
			printf("memoria[%d]=%d   ", ii, memoria[ii]);
			if ((ii % 5) == 0)printf("\t\n");
		}
		printf("Nr de ciclos executados =%d \n", nr_ciclos);
	
	exit(0);
} 
