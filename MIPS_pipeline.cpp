#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  PC;
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  PC;
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  

    bool        isBranch;
};

struct MEMStruct {
    bitset<32>  PC;
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;

    bool        isBranch;    
    bool        is_I_type;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read state.IF.Instr memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }

        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<(bool)state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}

class ALU
{
  public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bool ALUOP, bitset<32> oprand1, bitset<32> oprand2) {
      // TODO: implement!
      unsigned long result;

      if(ALUOP == 1){
          result = oprand1.to_ulong() + oprand2.to_ulong(); // addu
      }
      else {
          result = oprand1.to_ulong() - oprand2.to_ulong(); // subu
      }

      bitset<32> res((int)result);
      ALUresult = res;
      return ALUresult;
    }
};


bitset<32> PC_Adder (bitset<32> PC) {
    
    PC = bitset<32> (PC.to_ulong()+4);
    return PC;

    }




int main() {

    ALU myALU;
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    int cycle = 0;
    stateStruct state, newState;    

// Init State
    //WBStruct
    state.WB.Wrt_data = bitset<32> (0);
    state.WB.Rs = bitset<5> (0);
    state.WB.Rt = bitset<5> (0);
    state.WB.Wrt_reg_addr = bitset<5> (0);
    state.WB.wrt_enable = 0;
    state.WB.nop = 1;
    
    //MemStruct 
    state.MEM.is_I_type = 0;
    state.MEM.isBranch = 0;
    state.MEM.PC = bitset<32> (0);
    state.MEM.ALUresult = bitset<32> (0);
    state.MEM.Store_data = bitset<32> (0);
    state.MEM.Rs = bitset<5> (0);
    state.MEM.Rt = bitset<5> (0);
    state.MEM.Wrt_reg_addr = bitset<5> (0);
    state.MEM.wrt_enable = 0;
    state.MEM.wrt_mem = 0;
    state.MEM.rd_mem = 0;
    state.MEM.nop = 1;

    //EXstruct
    state.EX.isBranch = 0;
    state.EX.PC = bitset<32> (0);
    state.EX.Read_data1 = bitset<32> (0);
    state.EX.Read_data2 = bitset<32> (0);
    state.EX.Imm = bitset<16> (0);
    state.EX.Rs = bitset<5> (0);
    state.EX.Rt = bitset<5> (0);
    state.EX.Wrt_reg_addr = bitset<5> (0);
    state.EX.wrt_enable = 0;
    state.EX.alu_op = 1;
    state.EX.is_I_type = 0;
    state.EX.wrt_mem = 0;
    state.EX.rd_mem = 0;
    state.EX.nop = 1;    

    //IDStruct ID;
    state.ID.PC = bitset<32> (0);
    state.ID.Instr = bitset<32> (0);
    state.ID.nop = 1;

    //IFStruct IF;
    state.IF.PC = bitset<32> (0);    
    state.IF.nop = 0;    

// Init newState
    //WBStruct
    newState.WB.Wrt_data = bitset<32> (0);
    newState.WB.Rs = bitset<5> (0);
    newState.WB.Rt = bitset<5> (0);
    newState.WB.Wrt_reg_addr = bitset<5> (0);
    newState.WB.wrt_enable = 0;
    newState.WB.nop = 1;

    //MemStruct
    newState.MEM.is_I_type = 0;
    newState.MEM.isBranch = 0;
    newState.MEM.PC = bitset<32> (0);
    newState.MEM.ALUresult = bitset<32> (0);
    newState.MEM.Store_data = bitset<32> (0);
    newState.MEM.Rs = bitset<5> (0);
    newState.MEM.Rt = bitset<5> (0);
    newState.MEM.Wrt_reg_addr = bitset<5> (0);
    newState.MEM.wrt_enable = 0;
    newState.MEM.wrt_mem = 0;
    newState.MEM.rd_mem = 0;
    newState.MEM.nop = 1;

    //EXstruct
    newState.EX.isBranch = 0;
    newState.EX.PC = bitset<32> (0);
    newState.EX.Read_data1 = bitset<32> (0);
    newState.EX.Read_data2 = bitset<32> (0);
    newState.EX.Imm = bitset<16> (0);
    newState.EX.Rs = bitset<5> (0);
    newState.EX.Rt = bitset<5> (0);
    newState.EX.Wrt_reg_addr = bitset<5> (0);
    newState.EX.alu_op = 1;
    newState.EX.is_I_type = 0;
    newState.EX.wrt_enable = 0;
    newState.EX.wrt_mem = 0;
    newState.EX.rd_mem = 0;
    newState.EX.nop = 1;

    //IDStruct ID;
    newState.ID.PC = bitset<32> (0);
    newState.ID.Instr = bitset<32> (0);
    newState.ID.nop = 1;

    //IFStruct IF;
    newState.IF.PC = bitset<32> (0);
    newState.IF.nop = 0;


    bitset<32> RD2 = bitset<32> (0); //used in EX
    string imm_adrStr, opcode;
    char instrType;
    bool isLoad, isStore, isBranch;
    int endCount=6;

    while (1) {

        /* --------------------- WB stage --------------------- */
        

        
        if(state.WB.nop == 0) {

	    //wbMux();
	    if (state.WB.wrt_enable){
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);

                //Pretend forwarding mux		
                if (state.WB.Wrt_reg_addr == state.EX.Rs) state.EX.Read_data1 = newState.WB.Wrt_data; //state.EX.nop = 1;}
                if (state.WB.Wrt_reg_addr == state.EX.Rt) state.EX.Read_data2 = newState.WB.Wrt_data; //state.EX.nop = 1;}
	    }

	}

        /* --------------------- MEM stage --------------------- */
      
        if(state.MEM.nop == 0) {

            //I think something is going wrong in here
            
            if (state.MEM.wrt_mem) {
            myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
	    }
	    
	    
	    if (state.MEM.rd_mem) {
	    newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            } else newState.WB.Wrt_data = state.MEM.ALUresult;

            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;

            //Pretend forwarding mux
            if (state.MEM.wrt_mem) { state.EX.Read_data1 = newState.WB.Wrt_data; state.ID.nop = 1; state.IF.nop = 1; }
	    else if (state.MEM.Wrt_reg_addr == state.EX.Rs && !state.MEM.is_I_type && !state.MEM.wrt_mem && !state.MEM.isBranch) state.EX.Read_data1 = newState.WB.Wrt_data;
            if (state.MEM.wrt_mem) { state.EX.Read_data1 = newState.WB.Wrt_data; state.ID.nop = 1; state.IF.nop = 1; }
	    else if (state.MEM.Wrt_reg_addr == state.EX.Rt && !state.MEM.is_I_type && !state.MEM.wrt_mem && !state.MEM.isBranch) state.EX.Read_data2 = newState.WB.Wrt_data;


	}	
	newState.WB.nop = state.MEM.nop;

        /* --------------------- EX stage --------------------- */
     
        if(state.EX.nop == 0) {

            //I_TypeMux();
            if (state.EX.is_I_type == 1 ) {  }
	    else {
            
                RD2= state.EX.Read_data2;
	    }

	    if (state.EX.wrt_mem == 0 || state.EX.rd_mem == 0) {
                //ALU();
                newState.MEM.ALUresult = myALU.ALUOperation(state.EX.alu_op, state.EX.Read_data1, RD2);

		//Place holder so the Testbench1 diff will shut up
                newState.MEM.Store_data = RD2;
	    } else {
		newState.MEM.ALUresult = state.EX.Read_data1;
	    }

        if (state.EX.wrt_mem == 1 || state.EX.rd_mem == 1) {
            newState.MEM.Store_data = myRF.readRF(state.EX.Rt);
        }



        newState.MEM.Rs = state.EX.Rs;
        newState.MEM.Rt = state.EX.Rt;
        newState.MEM.rd_mem =  state.EX.rd_mem;
        newState.MEM.wrt_mem = state.EX.wrt_mem;
        newState.MEM.PC = state.EX.PC;
        newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
        newState.MEM.wrt_enable = state.EX.wrt_enable;
	newState.MEM.isBranch = state.EX.isBranch;
        newState.MEM.is_I_type = state.EX.is_I_type;

	}
        newState.MEM.nop = state.EX.nop;
        /* --------------------- ID stage --------------------- */

        if(state.ID.nop == 0) {

            

            opcode = state.ID.Instr.to_string().substr(0,6);
            
	    cout << opcode << endl;

            //Decoder();
            if (opcode == "000000") { instrType = 'R'; }
	    else if (opcode == "111111") { instrType = 'C'; }
            else { instrType = 'I'; }
    	
            isLoad = (opcode == "100011");
            isStore = (opcode == "101011");
            isBranch = (opcode == "000100");
            newState.EX.wrt_enable = (!isStore && !isBranch);
           	//isEq = ???:Q?????????????????????????????????????????????;
    
            //First if statement is for debug purposes???
            if (isLoad == true || isStore == true || state.ID.Instr[1] == 0) { //Instr[1] == 1 means addu
    
                    newState.EX.alu_op = 1;
    
            } else { //means subu
    
                    newState.EX.alu_op = 0;
    
            }
    	
            if (instrType == 'R'){
               
                newState.EX.is_I_type = false;

                newState.EX.Rs[0] = state.ID.Instr[21];
                newState.EX.Rs[1] = state.ID.Instr[22];
                newState.EX.Rs[2] = state.ID.Instr[23];
                newState.EX.Rs[3] = state.ID.Instr[24];
                newState.EX.Rs[4] = state.ID.Instr[25];
        
                newState.EX.Rt[0] = state.ID.Instr[16];
                newState.EX.Rt[1] = state.ID.Instr[17];
                newState.EX.Rt[2] = state.ID.Instr[18];
                newState.EX.Rt[3] = state.ID.Instr[19];
                newState.EX.Rt[4] = state.ID.Instr[20];
        
                newState.EX.Wrt_reg_addr[0] = state.ID.Instr[11];
                newState.EX.Wrt_reg_addr[1] = state.ID.Instr[12];
                newState.EX.Wrt_reg_addr[2] = state.ID.Instr[13];
                newState.EX.Wrt_reg_addr[3] = state.ID.Instr[14];
                newState.EX.Wrt_reg_addr[4] = state.ID.Instr[15];

		//Place holder so the Testbench1 diff will shut up
		 for (int i=0; i<16; i++){
                   newState.EX.Imm[i] = state.ID.Instr[i];
		 }

                newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);

            } else if (instrType == 'I') {

                newState.EX.is_I_type = true;

                newState.EX.Rs[0] = state.ID.Instr[21];
                newState.EX.Rs[1] = state.ID.Instr[22];
                newState.EX.Rs[2] = state.ID.Instr[23];
                newState.EX.Rs[3] = state.ID.Instr[24];
                newState.EX.Rs[4] = state.ID.Instr[25];

		//Rt and Wrt_reg_addr are same in this case
                newState.EX.Rt[0] = state.ID.Instr[16];
                newState.EX.Rt[1] = state.ID.Instr[17];
                newState.EX.Rt[2] = state.ID.Instr[18];
                newState.EX.Rt[3] = state.ID.Instr[19];
                newState.EX.Rt[4] = state.ID.Instr[20];
                //Read previous comment
                newState.EX.Wrt_reg_addr[0] = state.ID.Instr[16];
                newState.EX.Wrt_reg_addr[1] = state.ID.Instr[17];
                newState.EX.Wrt_reg_addr[2] = state.ID.Instr[18];
                newState.EX.Wrt_reg_addr[3] = state.ID.Instr[19];
                newState.EX.Wrt_reg_addr[4] = state.ID.Instr[20];

                for (int i=0; i<16; i++){
                   newState.EX.Imm[i] = state.ID.Instr[i];
                } 	
	        
		newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
		newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt); // unsure if needed
            }
    
            //TODO something is either wrong here or in MEM
	    newState.EX.rd_mem = isLoad; //?true?false?;
            newState.EX.wrt_mem = isStore; //?true?false?;

	    newState.EX.isBranch = isBranch;

            newState.EX.PC = state.ID.PC;
           
	    
	    if (instrType == 'I' && newState.EX.Read_data1 == newState.EX.Read_data2) {
                if (newState.EX.Imm[15] == 0) {
                    imm_adrStr = "0000000000000000" + newState.EX.Imm.to_string();
                } else if (newState.EX.Imm[15] == 1) {
                    imm_adrStr = "1111111111111111" + newState.EX.Imm.to_string();
                }
                bitset<32> finalImm(imm_adrStr);
                RD2 = finalImm;
                RD2 <<= 2;
                //newState.MEM.PC = bitset<32> (RD2.to_long() + state.EX.PC.to_long());
                if (isBranch == 1) {
                    state.IF.PC = bitset<32> ((int32_t)RD2.to_ulong() + (int32_t)state.EX.PC.to_ulong());
                    newState.IF.PC = state.IF.PC;
                    state.ID.nop = 1;
                    state.EX.nop = 1;
                    state.MEM.nop = 1;
                    state.WB.nop = 1;
               }

            }
	     
	}
        newState.EX.nop = state.ID.nop;

        /* --------------------- IF stage --------------------- */

        //checkNop();
        //state.IF.InstrMemory();
       if (state.IF.nop == 0) {
	
		newState.ID.Instr = myInsMem.readInstr(state.IF.PC.to_ulong());
		newState.ID.nop = state.IF.nop;
	        newState.IF.nop = state.IF.nop;
               //PC_Adder();
               newState.IF.PC = PC_Adder(state.IF.PC);

               //Pass PC for later use TODO PC math
               newState.ID.PC = state.IF.PC;

	       if (myInsMem.readInstr(state.IF.PC.to_ulong()).to_string() == "11111111111111111111111111111111") {
		       newState.IF.nop = true;
		       newState.ID.nop = true;
                       newState.IF.PC = state.IF.PC;
	       }
       }

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
                
        //This needs fixing TODO
	if(myInsMem.readInstr(state.IF.PC.to_ulong()).to_string() == "11111111111111111111111111111111" && endCount == 6) endCount=5;
	if(endCount < 6) endCount--;
	if(endCount == 0) break;

        cycle++;

    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}

//thing to correct, memory gets passed if nop happens?

