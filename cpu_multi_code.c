#include <stdio.h>
#include <stdlib.h>

extern int memoria[];
extern int reg[];
extern char loop;
extern void ula (int a, int b, char ula_op, int *result_ula, char *zero, char *overflow);

char returnSignal = 0;

//variavel pra testes
char pausa;
void funcaoProximoEstadoExplicita(){};
void UnidadeControle(int IR, int *sc){
	//comeca com -1 = 111111
	int opcode = IR >> 26; 
	unsigned char op1=0, op0=0, op2=0, op3=0, op4=0, op5=0;
	unsigned char n0=0, n1=0, n2=0, n3=0, n4=0;

	/*opcode & 0011 1111*/
	opcode = opcode&0x3f; //filtrando para tratar negativo

	switch(*sc) {
		
		case 0:				 //Inicializador
			/*sc = 0x12020;	 Estado 0*/
			*sc  = 0x12020;	 //Estado 0
/* sao 20 sinais controle 
  High 19-18--16---14-----98---765-----4----3 2 1------0 Lower	
		0 1  00   01 00 0000   001     0    0 0 0      0
		  IREsc	   LerMemoria    AluB1 AluB0    EscReg RegDst
		*/
		break;

		case 0x12020:		 //Estado 0    
			*sc = 0x30;		 //Estado 1  ... Estado0->Estado1
		break;

		case 0x30:			//Estado 1		
		/*

		(add, sub, slt, and, or, lw, sw, beq e j),jal, jr, jalr, addi, andi e bne.

		00- RegDst0 (RegDst0) 
		01- RegDst1 (RegDst1) 
		02- EscReg (RegWrite)
		03- UALFonteA (ALUSrcA) 
		04- UALFonteB0 (ALUSrcB0) 
		05- UALFonteB1 (ALUSrcB1)
		06- UALOp0 (ALUOp0) 
		07- UALOp1(ALUOp1) 
		08- FontePC0 (PCSource0)
		09- FontePC1 (PCSource1) 
		10- PCEscCond (PCWriteCond) 
		11- PCEsc (PCWrite)
		12- IouD (IorD) 
		13- LerMem (MemRead) 
		14- EscMem (MemWrite)
		15- BNE (BNE) 
		16- IREsc (IRWrite) 
		17- MemParaReg0 (MemtoReg0)
		18- MemParaReg1 (MemtoReg1)
			
			Tabela de despacho(fetch) de acordo com o microcódigo 	
			decode 		     (2)
			exec   		     (6)
			branch 		     (8)
			jump   		 	 (9)								  
			bne		0x05	 (10)
			addi	0x08	 (11)
			Jal	    0X03	 (12)
			andi	0X0c	 (13)
			jr		0X14	 (14)
			jalr	0X15	 (15)
		*/		
			if(opcode == 0){
				*sc = 0x48;	//Estado 6 - Se for Tipo-R    --- Estado1 -> Estado6
			}
			else if(opcode == 0x2B || opcode == 0x23 || opcode == 0x08){
				*sc = 0x18;	//Estado 2 - Se for lw/sw/ addi  ---- Estado1 -> Estado2
			}
			else{
				switch(opcode){
					case 0x4:	
						//*sc = 0xD0C;		//Estado 8 beq 	   --- Estado1 -> Estado8
						*sc = 0x688;		//Estado 8 beq 	   --- Estado1 -> Estado8
						break;

					case 0x2:	
						*sc = 0x900;		//Estado 9 jump(j) --- Estado1 -> Estado9
						break;

					case 0x5:	
						*sc = 0x8248;		//Estado 10 bne    --- Estado1 -> Estado10
						break;

					case 0x08:	
						*sc = 0x4;			//Estado 11 addi   --- Estado1 -> Estado11
						break;

					case 0x03:	
						*sc = 0x20905;		//Estado 12 jal  --- Estado1 -> Estado12
						break;

					case 0x0C:	
						*sc = 0xD8;			//Estado 13 andi    --- Estado1 -> Estado13
						break;
					
					case 0x14:	
						*sc = 0xB00;	    //Estado 14 jr	--- Estado1 -> Estado14
						break;

					case 0x15:	
						*sc = 0x20B05;	    //Estado 15 jalr --- Estado1 -> Estado15
						break;
					
				}
				
			}
		break;

		case 0x18:	//Estado2 (lw ou sw)
			/*
			
			Tabela de despacho(fetch) de acordo com o microcódigo 

				lw 	 0x2B 	(3)
				sw 	 0x23 	(5)
				addi 0x08   (11)
				
			*/	
			if(opcode == 0x23) {
				*sc = 0x3000;	//Estado2 -> Estado3 LW
			}
			else if(opcode == 0x2B){
				*sc = 0x5000;	//Estado2 -> Estado5 SW
			}else {
				*sc = 0x4;	    //Estado2 -> Estado11 ADDI
			}
			break;

		case 0x3000:	
			*sc = 0x40004; 	   //Estado3 -> Estado4
			break;

		case 0x40004:	
			*sc = 0x12020;	   //Estado4 -> Estado0
			break;

		case 0x5000:	
			*sc = 0x12020;	   //Estado5 -> Estado0
			break;

		case 0x48:			   //Estado6 -> Estado7
			*sc = 0x6;	
			break;

		case 0x6:	
			*sc = 0x12020;	   //Estado7 -> Estado0
			break;

		case 0x688:		
			*sc = 0x12020;    //Estado8 -> Estado0
			break;

		case 0x900:	 
			*sc = 0x12020;	  //Estado9 -> Estado0
			break;

		case 0x8248:	
			*sc = 0x12020;	 //Estado10 -> Estado0
			break;

		case 0x4:	
			*sc = 0x12020;	////Estado11 -> Estado0
			break;

		case 0x20905:	
			*sc = 0x12020;	//Estado12 -> Estado0
			break;

		case 0xD8:		//Estado13 addi execute -> Estado11 write back result
			*sc = 0x4;	
			break;

		case 0xB00:		   //Estado14 -> Estado0
			*sc = 0x12020;	
			break;

		case 0x20B05:	      //Estado15 -> Estado 0
			*sc = 0x12020;	
			break;
	}

	//teste
	printf("estado: %d\n", *sc);

}



void Busca_Instrucao(int sc, int PC, int ALUOUT, int IR, int A, int B, int *PCnew, int *IRnew, int *MDRnew){
	
	char zero, overflow;
	/*19 18 17 16 15  0
	   0 1  0  0  0...0 												*/
	//if((sc & 0x40000) !=0) {			 Se IRWrite == 1 'bit-18' PUZZY
	if((sc & 0x10000) !=0) {			 //Se IRWrite == 1 'bit-16' Pelanza
		//if((sc & 0x2000) == 0) {		 Se IorD == 0 'bit-13', a memória é acessada na posição de PC
		if((sc & 0x1000) == 0) {		 //Se IorD == 0 'bit-12', a memória é acessada na posição de PC
			*IRnew = memoria[PC/4];
		}
		else{							//Se IorD == 1, a memória é acessada na posição de ALUout
			*IRnew = memoria[ALUOUT/4];
		}
		*MDRnew = *IRnew << 16;			//????
		*MDRnew = *MDRnew >> 16;
		ula(PC, 0x4, 0x2, PCnew, &zero, &overflow);
	}
}



void Decodifica_BuscaRegistrador(int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew){
	
	char zero, overflow;
	/*igual */
	if(sc==0x30) {	//Se Estado1 (decode)
		if(IR == 0) {
			loop = 0;
			printf("HALT!\n");
			returnSignal = 1;
			return;
		}
		else {
			*Anew = IR >> 21;		//*Anew recebe os 5 bits referentes ao registrador RS
			*Anew = *Anew & 0x1f;

			*Bnew = IR >> 16;		//*Anew recebe os 5 bits referentes ao registrador RT
			*Bnew = *Bnew & 0x1f;

			IR = IR << 16;
			IR = IR >> 14;
			ula(PC, IR, 0x2, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT = PC + 4;
		}
	}
}



void Execucao_CalcEnd_Desvio(int sc, int A, int B, int IR, int PC, int ALUOUT, int *ALUOUTnew, int *PCnew){

	char zero, overflow;
	int op;
	/* Valor IMEDIATO 16-bits*/
	int VI = IR << 16;
	VI = VI >> 16;
	 
	if(returnSignal == 1) {
		return;
	}

	//if(sc == 0x88){		//Se for tipo-r pegando campo funcao 0x3f = 0011 1111
	if(sc == 0x48){		//Se for tipo-r pegando campo funcao 0x3f = 0011 1111
		/*6 bits campo funcao*/
		op = (IR & 0x3f);
		switch(op) {
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
	/*Estado 3 Tipo-I LW/SW */
	else if(sc == 0x18) {	/*Pelanza-lw/sw*/
		ula(reg[A], VI, 0x2, ALUOUTnew, &zero, &overflow);	//Atualiza o valor de ALUOUT
	}				   //add
	/*
	else if(sc == 0xCF4) {	//BLTZ
		ula(reg[A], 0, 0x7, ALUOUTnew, &zero, &overflow);
		
		if(*ALUOUTnew == 1) {
			*PCnew = ALUOUT;
		}
	}
	*/
	//else if(sc == 0xD0C) } puzzy-BEQ
	else if(sc == 0x688) {	/*Pelanza-BEQ aluOp=6 sub*/
		ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);	
		if(zero == 1) {
			*PCnew = ALUOUT;
		}
	}
	/*else if(sc == 0xD08) {	puzzy-BNE*/	
	else if(sc == 0x8248) { /*Pelanza-BNE*/
		ula(reg[A], reg[B], 0x6, ALUOUTnew, &zero, &overflow);
		if(zero == 0) {
			*PCnew = ALUOUT;
		}
	}
	/*else if(sc == 0x1A00) {	puzzy-j*/
	else if(sc == 0x900) {	/*Pelanza-J*/
		VI = IR&0x3ffffff;	//realiza a extensão de sinal
		VI = VI << 2;
		*PCnew = VI;
	} 
	/* trocar por addi
	else if(sc == 88) {		//ori
		ula(reg[A], VI, 0x1, ALUOUTnew, &zero, &overflow);
	}
	*/
	else if(sc == 0x4) {	/*Pelanza-Addi rs rt imediate AluOp=02*/
		ula(reg[A], VI, 0x2, ALUOUTnew, &zero, &overflow);
	}
	else if(sc == 0xD8) {	/* Pelanza-Andi rs rt imediatte AluOp=00*/
		ula(reg[A], VI, 0x0, ALUOUTnew, &zero, &overflow);
	}
	
	else if(sc == 0x20905) {		/* Pelanza-JAL imediatte AluOp=00*/
		VI = IR&0x3ffffff;	//realiza a extensão de sinal
		VI = VI << 2;
		*PCnew = VI;
		reg[31] = PC;
	}

	else if(sc == 0xB00) {	/* Pelanza-JR rs */
		*PCnew = reg[A];
	}

	else if(sc == 0x20B05) {	/* Pelanza-JALR rs rd */
		reg[B] = IR << 16;
		*PCnew = reg[A];
	}
	
}



void EscreveTipoR_AcessaMemoria(int sc, int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew){
	int RD;
	if(returnSignal==1) {
		return;
	}
	//if(sc == 3) {	//7
	if(sc == 6) {	//7
		if(IR >> 27 == 0) {	//Tipo-R
			RD = IR >> 11;
			RD = RD&0x1f;
			reg[RD] = ALUOUT;
		}
		else{	//Tipo-I
			reg[B] = ALUOUT;
		}
		printf("\t\tresultado da ula: %d\n", ALUOUT);
	}
	else if(sc == 0x3000) {	//se LW
		(*MDRnew) = (memoria[ALUOUT/4]) << 16;	//LW
		(*MDRnew) = (*MDRnew) >> 16;	//LW
		printf("----------ALUOUT: %d\n", ALUOUT);
		printf("MDRnew: %d\n", *MDRnew);
	}
	else if(sc == 0x5000) {	//5 se SW
		memoria[ALUOUT/4] = reg[B];	//SW
	}
}



void EscreveRefMem(int sc, int IR, int MDR, int ALUOUT){
	if(returnSignal == 1) {
		return;
	}
	int B = IR >> 16;		//*Anew recebe os 5 bits referentes ao registrador RT
	B = B & 0x1f;
	if(sc == 0x40004) { //estado 4
		reg[B] = MDR;
	}
}



