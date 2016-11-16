/*
 * 
 * UNIVERSIDADE DO OESTE DE SANTA CATARINA - UNOESC
 * CAMPUS CHAPECÓ
 * ENGENHARIA DA COMPUTAÇÃO - 6º PERÍODO
 * PROFESSOR: JACSON LUIZ MATTE
 * DISCIPLINA: COMPILADORES
 * ACADÊMICO: FELIPE ANDRÉ ZEISER
 * 
 * TRABALHO DE SIMULAÇÃO DO 'FIRST', 'FOLLOW' 
 * 		 E 'TABELA PREDITIVA'
 * 
 * 
*/
#include <bits/stdc++.h>

using namespace std;



//utilize o seguinte comando de compilação g++ first.cpp -o first
//e para executar ./first entrada.txt saida.txt



/* ************************ 
 * 		   FIRST
 * ************************
*/

//string utilizada como auxiliar na chamada de funcoes e comparacoes
string aux;

string prodInicial;

//guarda o nome do arquivo de entrada
char *arquivoIn;

//guarda o nome do arquivo de saida
char *arquivoOut;
ofstream saida;

//guarda simbolos nao terminais e seus first
map< string, string > first;

//guarda a linguagem de leitura
map< string, string> linguagem;

//guarda se pode ter produção vazia
map< string, bool > vazio;

//mapa auxiliar para uso na tabela preditiva
map<string, vector< pair<string, string> > > firstTabela;
//mapa auxiliar para uso na tabela preditiva
map<string, vector< pair<string, string> > > followTabela;

//funcao responsavel por carregar o arquivo de entrada
int carregaArquivo(){
	
	bool producao = true;
	
	//variavel que armazena o caractere de leitura
	char entrada;
	
	//ponteiro de leitura
	FILE *leitura;
	
	//strings responsaveis pelo armazenamento dos terminais 
	//e nao terminais atuais na hora da leitura
	string naoTerminal, terminal; naoTerminal = ""; terminal = "";
	
	//flag responsável pelo controle dos nao terminais e terminais
	bool troca = false;
	
	//abre o arquivo
	leitura = fopen(arquivoIn, "r");
	if(leitura==NULL){
		printf("Erro ao abrir o arquivo de Entrada\n");
		return 0;
	}	
	
	//posiciona o ponteiro de leitura na posicao inicial do arquivo 
	fseek(leitura,0,SEEK_SET);	
	
	while(1){
		//verifica se é o fim do arquivo
		if(feof(leitura)) break;
		
		//lê caractere a caractere do arquivo
		fread(&entrada,sizeof(char),1,leitura);
		
		//verifica se é o fim de um nao terminal
		if(entrada == '-') troca = true;
		
		//verifica se continua sendo um nao terminal
		if(!troca&&isalpha(entrada)) naoTerminal += entrada;
		
		//verifica se é um terminal
		else if(troca&&(isalpha(entrada)||entrada=='|'||entrada=='&')) terminal += entrada;
		
		//verifica se acabou a sequencia de nao terminal + terminal
		else if(entrada=='\n'){
			if(producao){
				prodInicial = naoTerminal;
				producao = false;
			}
			troca = false;
			linguagem[naoTerminal] = terminal;
			naoTerminal.clear();
			naoTerminal = "";
			terminal.clear();
			terminal = "";
			
		}
	}
	
	//fecha o arquivo
	fclose(leitura);
}

//recebe um char e verifica se o mesmo esta contido no mapa
//se sim ele é Nao terminal se nao é terminal
bool isTerminal(char valor){
	string terminal; terminal = ""; terminal+=valor;
	if(linguagem.count(terminal)==0) return 1;
	return 0;
}


//verifica se todas os terminais daquela producao
//podem ser vazios
bool isEmpty(string expAtual){
	int tam = expAtual.length();
	string aux;
	for(int i=0;i<tam;i++){
		aux = expAtual[i];
		if(vazio[aux] == 0) return 0;
	}return 1;
}


//função responsável por gerar os first da linguagem
string geraFirst(string atual){
	bool producao = true;
	string expAtual = "";
	string firstAtual = "";
	if(first.count(atual)==0){
		first[atual] = "";
		vazio[atual] = 0;
		string expressao= linguagem[atual];
		for(int i=0;i<expressao.length();i++){
			if(expressao[i]!='|')
				expAtual+=expressao[i];
			//verifica se é um caractere minusculo 
			//e se pode realizar a producao
			if(isTerminal(expressao[i])&&producao&&expressao[i]!='&'&&expressao[i]!='|'){
				//adiciona o caractere a producao first
				first[atual] += expressao[i];
				firstAtual += expressao[i];
				//seta a producao para falso
				producao = false;				
			}
			//verifica se é um caractere maiusculo 
			//e verifica se é possivel criar outra producao 
			else if(!isTerminal(expressao[i])&&producao&&expressao[i]!='&'){
				aux=expressao[i];
				//chamada da função novamente para a producao de 'aux'
				if(atual!=aux){
					aux = geraFirst(aux);
					firstAtual += aux;
					first[atual] += aux;
				}
				aux=expressao[i];
				//seta a producao para falso
				producao = false;
				//ve se a producao atual pode ser vazia,
				//se sim passa a producao para verdadeira
				if(vazio[aux])  producao = true;
			}
			//ve se pode criar outra producao
			else if(expressao[i]=='|'){
				//verifica se a producao atual permite produzir vazio
				if(isEmpty(expAtual)&&vazio[atual]==0){ vazio[atual] = 1; firstAtual+='&';}
				//guarda a producao atual no mapa auxiliar para uso na tabela preditiva
				firstTabela[atual].push_back( make_pair(expAtual, firstAtual) );
				expAtual.clear(); expAtual="";
				firstAtual.clear(); firstAtual="";
				producao = true;
			}
			//ve se existe a produção vazia, se sim
			//flega o map de vazio para true
			else if(expressao[i]=='&'){
				vazio[atual] = true;
				firstAtual+='&';
			}
		}
	}
	//verifica se a producao atual permite produzir vazio
	if(isEmpty(expAtual)&&vazio[atual]==0){ vazio[atual] = 1; firstAtual+='&';}
	//guarda a producao atual no mapa auxiliar para uso na tabela preditiva
	if(firstAtual!=""){
		firstTabela[atual].push_back( make_pair(expAtual, firstAtual) );
		expAtual.clear(); expAtual="";
		firstAtual.clear(); firstAtual="";
	}
	return first[atual];	
}

//guarda todos os terminais para uso na tabela preditiva
string terminais;

//função responsavel por retirar os terminais duplicados do first e da string terminais
void retiraDuplicados(){
	
	map<string,string>::iterator i;
	string expressao, retira;
	bool contido[1000], contidoTerminais[1000];
	int j;
	terminais = "";
	memset(contidoTerminais,0,sizeof contidoTerminais);
	
	//percorre o mapa de first
	for(i = first.begin();i!=first.end();i++){
		//seta o vetor de flag de caracteres
		memset(contido, 0, sizeof contido);
		//pega a expressao referente ao valor do map
		expressao = i->second;
		//percorre a expressao
		retira.clear();
		retira="";
		for(j=0;j<expressao.length();j++){
			//verifica se aquele caractere nao foi impresso ainda
			if(!contido[expressao[j]]){
				//se nao foi imprime
				retira+=expressao[j];
				//seta para verdadeiro o vetor de contido
				contido[expressao[j]] = 1;
				
			}
			//ve se o caractere atuasl esta contido na string de terminais
			//se nao adiciona e flega o vetor de contido
			if(!contidoTerminais[expressao[j]]){
				terminais+=expressao[j];
				contidoTerminais[expressao[j]] = 1;
			}
		}
		//verifica se pode produzir o estado vazio
		if(vazio[i->first]) retira+='&';
		first[i->first] = retira;
	}
	terminais+="&$";
}

//função que imprime o first
void imprimeFirst(){
	
	map<string,string>::iterator i;
	string expressao;
	int j;
	
	//percorre o mapa de first
	for(i = first.begin();i!=first.end();i++){
		saida<<i->first<<" => { ";
		//pega a expressao referente ao valor do map
		expressao = i->second;
		//percorre a expressao
		for(j=0;j<expressao.length();j++){
			if(j!=0) saida<<", ";
			saida<<expressao[j];
		}
		saida<<" }"<<endl;
	}
}


/* ************************ 
 * 		  FOLLOW
 * ************************
*/

//mapa responsavel por armazenar o follow
map<string, string> follow;

//funcao responsavel por gerar o follow
string geraFollow(string atual){
	int j;
	map<string, string>::iterator i;
	string expressao, aux, followAtual, expAtual;
	bool producao = false;
	followAtual="";
	expAtual="";
	//verifica se o mesmo já nao foi calculado
	//se nao foi calculado, calcula
	if(follow.count(atual) == 0){
		follow[atual] = "";
		//percorre todas as producoes buscando o follow atual
		for(i=linguagem.begin();i!=linguagem.end();i++){
			//ve a producao atual nao é vazia
			if(i->first!=""){
				//guarda a producao atual na string expressao
				expressao = i->second;
				//percorre caractere a caractere da producao atual
				for(j=0;j<expressao.length();j++){
					aux.clear();aux = "";
					aux +=expressao[j];
					//se for igual '|' adiciona o follow da producao atual para o
					//mapa auxiliar do follow para uso na tabela preditiva
					if(expressao[j]=='|'){
						if(atual!="")
							followTabela[atual].push_back(make_pair(expAtual,followAtual));
						expAtual.clear();followAtual.clear();
						expAtual="";followAtual="";
					}else
						expAtual+=expressao[j];
					//verifica se pode produzir o follow
					if(producao){
						//se sim verifica se nao é '|'
						if(expressao[j]!= '|'){
							//ve se é terminal se for adiciona e seta a producao
							//para falso
							if(isTerminal(expressao[j])){
								follow[atual]+=expressao[j];
								followAtual+=expressao[j];
								producao = false;
							}
							//se nao adiciona o first do nao terminal atual para o follow
							//da string atual
							else{
								follow[atual]+=first[aux];
								followAtual+=first[aux];
								if(vazio[aux] == 0) producao = false; 
							}
						}
						//se for igual '|'
						//ve se o o follow da producao atual ja foi calculado
						//se nao calcula e adiciona o no follow da string atual
						else{
							if(follow.count(i->first)>0){
								follow[atual]+=follow[i->first];
								followAtual+=follow[i->first];
							}
							else{
								follow[atual]+=geraFollow(i->first);
								followAtual+=geraFollow(i->first);
							}
							producao = false;
						}
					}
					//ve se o nao terminal atual é igual a string atual
					// se sim permite produzir o follow de atual na proxima iteração
					if(expressao[j]==atual[0])
						producao = true;
				}
				//ve se a producao saiu como verdadeira se sim adiciona no mapa
				//auxiliar de follow para uso na tabela preditiva 
				if(producao){
					if(follow.count(i->first)>0){
						follow[atual]+=follow[i->first];
						followAtual+=follow[i->first];
					}else{
						follow[atual]+=geraFollow(i->first);
						followAtual+=geraFollow(i->first);
					}
					producao = false;
				}
				if(atual!=""){
					followTabela[atual].push_back(make_pair(expAtual,followAtual));
				}
				expAtual.clear();followAtual.clear();
				expAtual="";followAtual="";
			}
		}
		
	}
	if(follow[atual] == "") follow[atual]+='$';
	return follow[atual];
}

//funcao responsavel por limpar os caracteres repetidos do follow
void limpaFollow(){
	
	map<string,string>::iterator i;
	string expressao, retira;
	bool contido[1000];
	int j;
	
	//percorre o mapa de follow
	for(i = follow.begin();i!=follow.end();i++){
		//seta o vetor de flag de caracteres
		memset(contido, 0, sizeof contido);
		//pega a expressao referente ao valor do map
		expressao = i->second;
		//percorre a expressao
		retira.clear();
		retira="";
		for(j=0;j<expressao.length();j++){
			//verifica se aquele caractere nao foi impresso ainda
			if(!contido[expressao[j]]&&expressao[j]!='&'){
				//se nao foi imprime
				retira+=expressao[j];
				//seta para verdadeiro o vetor de contido
				contido[expressao[j]] = 1;
			}
		}
		follow[i->first] = retira;
	}
}

//funcao responsavel por imprimir o follow
void imprimeFollow(){
	
	map<string,string>::iterator i;
	string expressao;
	int j;
	
	//percorre o mapa de follow
	for(i = follow.begin();i!=follow.end();i++){
		if(i->first!=""){
			saida<<i->first<<" => { ";
			//pega a expressao referente ao valor do map
			expressao = i->second;
			//percorre a expressao
			for(j=0;j<expressao.length();j++){
				if(j!=0) saida<<", ";
				saida<<expressao[j];
			}
			saida<<" }"<<endl;
		}
	}
}

/* ************************ 
 * ANÁLISE PREDITIVA TABULAR
 * ************************
*/

//mapa que guarda a tabela preditiva
map<string, map<char, string> > tabela;
int maior = -1;

//ve se existe a possibilidade de se produzir vazio
//a partir da producao atual
bool vazioTabela(string atual){
	int x = atual.length();
	for(int i=0;i<x;i++)
		if(atual[i]=='&') return 1;
	return 0;
}

//funcao responsavel por criar a tabela preditiva
void analise(){
	//iteradores utilizados para percorrer os mapas do first e follow respectivamente
	map<string, vector< pair<string, string> > >::iterator it;
	map<string, vector< pair<string, string> > >::iterator it2;
	bool contido[1000];
	int i,j;
	//percorre a tabela do first
	for(it=firstTabela.begin();it!=firstTabela.end();it++){
		memset(contido,0,sizeof contido);
		//cria um mapa auxiliar para ser adiciona no final na posição it->first
		//ou seja para cada nao terminal tenho um mapa diferente
		map<char, string> auxiliarTabela;
		//percorre as producoes do vetor salvas no mapa atual
		for(i=0;i<it->second.size();i++){
			//pega a producao atual na posicao i
			for(j=0;j<it->second[i].second.length();j++){
				//ve se a mesma ainda nao foi produzida
				if(!contido[it->second[i].second[j]]){
					//ve se nao é vazia
					if(it->second[i].second[j]!='&'){
						//salva no mapa auxiliar para depois ser adicionado no mapa final
						auxiliarTabela[it->second[i].second[j]] = it->second[i].first;
						contido[it->second[i].second[j]] = true;
						if((int)it->second[i].first.length()>maior){
							maior = (int)it->second[i].first.length();
						}
					}
					//se for vazia adiciona a producao atual nos terminais do follow
					else{
						for(int k=0;k<follow[it->first].length();k++){
							//verifica se a mesma ainda nao foi produzida
							if(!contido[follow[it->first][k]]){
							//salva no mapa auxiliar para depois ser adicionado no mapa final
								auxiliarTabela[follow[it->first][k]] = it->second[i].first;
								contido[follow[it->first][k]] = true;
								if((int)it->second[i].first.length()>maior){
									maior = (int)it->second[i].first.length();
								}
							}
						}
					}
				}
			}
		}
		//salva o mapa auxiliar na posicao it->first
		tabela[it->first] = auxiliarTabela;
	}
}

//funcao utilizada na tabela para dar um 'alinhamento'
void alinha(int tam, int atual){
	for(int i=0;i<(tam+2)-atual;i++) saida<<" ";
}

int main(int argc, char *argv[]){
	map<string, string>::iterator it;
	if (argc < 3) {
		printf("Entre com o arquivo de < entrada > e < saida >\n");
		return 0;
	}
	//pega o nome do arquivo de entrada 
	arquivoIn = argv[1];
	//carrega o arquivo
	carregaArquivo();
	//pega a string base para a chamada da função de calculo dos first
	string producao = "S";
	//chama a função
	producao = geraFirst(prodInicial);
	//verifica se existem produções ainda não calculados
	for(it=linguagem.begin();it!=linguagem.end();it++){
		char c = it->first[0];
		if(first.count(it->first)==0&&isalpha(c)){
			producao = geraFirst(it->first);
		}
	}
	
	//pega o nome do arquivo de saida
	arquivoOut = argv[2];
	saida.open(arquivoOut);
	
	//retira caracteres duplicados 
	retiraDuplicados();
	saida<<"******************"<<endl;
	saida<<"      FIRST       "<<endl;
	saida<<"******************"<<"\n\n";
	//imprime o first
	imprimeFirst();
	
	follow[prodInicial] = "$";
	//calcula o follow
	for(it=linguagem.begin();it!=linguagem.end();it++){
		string c = it->first;
		if(follow.count(it->first)==0){
			producao = geraFollow(it->first);
		}
	}
	//limpa o follow retirando duplicados
	limpaFollow();
	saida<<"\n";
	saida<<"******************"<<endl;
	saida<<"      FOLLOW      "<<endl;
	saida<<"******************"<<"\n\n";
	//imprime o follow
	imprimeFollow();
	
	//realiza o calculo da tabela preditiva
	analise();
	
	saida<<"\n";
	saida<<"******************"<<endl;
	saida<<" TABELA PREDITIVA "<<endl;
	saida<<"******************"<<"\n\n";
	
	//inicia a impressao da tabela preditiva imprimindo os terminais
	map<string, map<char,string> >::iterator it3;
	saida<<" ";
	for(int i=0;i<terminais.size();i++){
		if(terminais[i]!='&'){
			saida<<"|";
			alinha(maior+3/2,1);
			saida<<terminais[i];
			alinha(maior+3/2,1);
		}
	}saida<<"|\n";
	
	//imprime os nao terminais e as suas possiveis produções
	for(it3=tabela.begin();it3!=tabela.end();it3++){
		saida<<it3->first;
		for(int i=0;i<terminais.size();i++){
			if(terminais[i]!='&'){
				saida<<"|";
				
				if(it3->second[terminais[i]]!=""){
					alinha(maior/2,(int)it3->second[terminais[i]].length());
					saida<<it3->first<<"->";
					saida<<it3->second[terminais[i]];
				}else{
					alinha(maior+3/2,(int)it3->second[terminais[i]].length());
				}
				alinha(maior+3/2,1);
			}
		}
		saida<<"|"<<endl;
	}
	
	//fecha o arquivo de saida
	saida.close();
	return 0;
}
