#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

typedef struct Node {  
    int peso;
    unsigned char caractere;
    short right = -1;
    short left = -1;
} Node;

// Funções do Heap //

int parent(int i) { return (i - 1) / 2; }
int left(int i) { return (2 * i + 1); }
int right(int i) { return (2 * i + 2); }

void MinHeapify(int i, int heap_size, Node* heap) {
    int l = left(i);
    int r = right(i);
    int smallest = i;
    if (l < heap_size && heap[l].peso < heap[i].peso)
        smallest = l;
    if (r < heap_size && heap[r].peso < heap[smallest].peso)
        smallest = r;
    if (smallest != i) {
        swap(heap[i], heap[smallest]);
        MinHeapify(smallest, heap_size, heap);
    }
}

Node MinExtract(Node* heap, int* heap_size) {
    Node ans;
    swap(heap[0], heap[(*heap_size) - 1]);
    ans = heap[*heap_size - 1];
    MinHeapify(0, *heap_size - 1, heap);
    *heap_size = *heap_size - 1;
    return ans;
}

void insertNode(Node node, int* heap_size, Node* heap) {
    heap[*heap_size] = node;
    *heap_size = *heap_size + 1;
    MinHeapify(0, *heap_size, heap);
}

void criar_heap(unsigned long* v, Node* heap) {
    int j = 0;
    for (unsigned int i = 0; i < 256; i++) {
        if (v[i] > 0) {
            heap[j].caractere = i;
            heap[j].peso = v[i];
            ++j;
        }
    } MinHeapify(0, j, heap);
}

// Fim das Funções do Heap //

int ler_tamanho(ifstream &readArq) { // ler o número de bytes do arquivo
  int size = 0;
  while(true) {
    readArq.get();
    if(readArq.eof()) break;
    size++;
  } return size;
}

int contagem_peso(ifstream &arq, unsigned long* v) { // ler o número de caracteres diferentes e a frequência deles
  unsigned char c = arq.get();
  int size = 0;
  while (arq.good()) {
    if(v[c] == 0) ++size;
    ++v[c];
    c = arq.get();
  } return size;
}

void gerar_arvore(int* heap_size, Node* heap, Node *arvore, int i) { // gera a árvore de Huffman
  if (*heap_size <= 1) return;

  Node* x = new Node();
  Node* y = new Node();

  arvore[i++] = *x = MinExtract(heap, heap_size); // extrai o mínimo e coloca no vetor da árvore
  arvore[i++] = *y = MinExtract(heap, heap_size);

  insertNode({x->peso + y->peso, 0, (short)(i-1), (short)(i-2)}, heap_size, heap); // insere o nó no heap e o ordena novamento em heap
  gerar_arvore(heap_size, heap, arvore, i); // recursão
}

void gerar_codigos(char *codigos[256], Node arvore[], int arvore_i, string s) { // gera os códigos dos caracteres
  if(arvore[arvore_i].left == -1) {
    codigos[(unsigned char)arvore[arvore_i].caractere] = new char[s.length() + 1];
    strcpy(codigos[(unsigned char)arvore[arvore_i].caractere], s.c_str());
    return;
  }
  s.push_back('0');
  gerar_codigos(codigos, arvore, arvore[arvore_i].left, s);
  s.pop_back();
  s.push_back('1');
  gerar_codigos(codigos, arvore, arvore[arvore_i].right, s); 
}


void gravar_bytes(ofstream &arq, char *start, int n) { // grava os bytes no arquivo
  for (char *p = start; p != start + n; p++)  
    arq.put(*p);
}

void ler_bytes(ifstream &arq, char *start, int n) { // ler os bytes do arquivo
  unsigned char byte; // Variável auxiliar
  for (char *p = start; p != start + n; p++) {
    byte = arq.get();
    if(arq.eof()) break;
    *p = byte;
  }
}

void gravar_codigo(ofstream &arq, unsigned char &b, int &size_b, char codigo[]) { // grava os códigos nos arquivos, completando 8 bytes
  for (size_t i = 0; i < strlen(codigo); i++) {
    b <<= 1;
    if(codigo[i] == '1') b += 1;
    size_b++;
    if(size_b == 8) {
      arq.put(b);
      b = size_b = 0;
    }
  }
}

int buscar_caractere(Node arvore[], unsigned char &b, int &size_b, int i) {  // busca os caractere no vetor da arvore
  while (arvore[i].left != -1 && size_b > 0) {
    if(b >= (unsigned char)128)
      i = arvore[i].right;
    else 
      i = arvore[i].left; 
      
    b <<= 1; 
    size_b--; 
  }
  return i;
}

void comprimir(ifstream &readArq, ofstream &writeArq) {
  unsigned long v[256] = { 0 }; // vetor dos caracteres
  char *codigos[256] = {0}; // vetor dos códigos

  int size_arq = ler_tamanho(readArq); // tamanho do arquivo em bytes

  readArq.clear(); 
  readArq.seekg(0);
  // volta pro inicio do arquivo

  int qtd_caractere, // quantidade de caracteres diferentes no arquivo
      heap_size; // tamanho do heap 
  heap_size = qtd_caractere = contagem_peso(readArq, v);

  if(qtd_caractere > 1) { 
    Node* heap = new Node[heap_size]; // vetor do heap com o tamanho de caracteres diferentes
    criar_heap(v, heap); // organiza vetor em heap

    int arvore_size = qtd_caractere * 2 - 1;
    Node *arvore = new Node[arvore_size]; // vetor da árvore
    gerar_arvore(&heap_size, heap, arvore, 0); // gera a árvore de Huffman
    arvore[arvore_size-1] = MinExtract(heap, &heap_size); // extrai o nó restante do Heap para completar a árvore

    // começo do preludio
    gravar_bytes(writeArq, (char*)&qtd_caractere, sizeof(int)); // grava quantidade de caracteres diferentes
    for(int i = 0; i < arvore_size; i++) { // grava os nós da árvore
      gravar_bytes(writeArq, (char*)&arvore[i].left, sizeof(arvore[i].left)); // filho direito
      gravar_bytes(writeArq, (char*)&arvore[i].right, sizeof(arvore[i].right)); // filho esquerdo
      gravar_bytes(writeArq, (char*)&arvore[i].caractere, sizeof(arvore[i].caractere)); // caractere
    }

    gravar_bytes(writeArq, (char*)&size_arq, sizeof(int)); // grava o tamanho do arquivo em bytes

    readArq.clear(); 
    readArq.seekg(0); // volta pro início do arquivo
    // fim do preludio

    gerar_codigos(codigos, arvore, arvore_size - 1, ""); // gera os codigos dos caracteres

    unsigned char b = 0, // byte a ser manipulado
                  byte = 0; // byte lido no arquivo
    int size_b = 0;
    while (true) { // ler o arquivo novamente salvando os códigos no arquivo compactado
      byte = readArq.get();
      if(readArq.eof()) break;
      gravar_codigo(writeArq, b, size_b, codigos[byte]); // grava o códigos no arquivo
    }
    if(size_b > 0) { // preenche os restantes dos bytes, tem que ser 8
      b <<= 8 - size_b;
      writeArq.put(b);
    }
  } else if(qtd_caractere == 1) { // se tiver 1 caractere diferente
      gravar_bytes(writeArq, (char*)&size_arq, sizeof(int)); // grava o número de bytes iguais no arquivo
      char c;
      for (int i = 0; i < 256; i++)
        if(v[i]) { c = (char)i; break; }
      gravar_bytes(writeArq, (char*)&c, sizeof(c)); // grava esse caractere no arquivo
    }
}

void descomprimir(ifstream &readArq, ofstream &writeArq) {
  int size_arq = ler_tamanho(readArq); // ler o tamanho do arquivo a ser descompactado

  readArq.clear(); 
  readArq.seekg(0);
  // volta pro inicio do arquivo

  if(size_arq > 5) {
    int qtd_caractere;
    ler_bytes(readArq, (char*)&qtd_caractere, sizeof(qtd_caractere)); // ler quantidade de caracteres diferentes
    int arvore_size = 2 * qtd_caractere - 1; // tamanho da árvore
    Node *arvore = new Node[arvore_size];
    for (int i = 0; i < arvore_size; i++) { // ler os nós da árvore
      ler_bytes(readArq, (char*)&arvore[i].left, sizeof(arvore[i].left)); // filho esquerdo
      ler_bytes(readArq, (char*)&arvore[i].right, sizeof(arvore[i].right)); // filho direito
      ler_bytes(readArq, (char*)&arvore[i].caractere, sizeof(arvore[i].caractere)); // caractere
    }

    int size_f_arq; 
    ler_bytes(readArq, (char*)&size_f_arq, sizeof(size_f_arq)); // ler o tamanho do arquivo original em bytes

    unsigned char byte = 0; // byte lido do arquivo
    int b_size = 0, // tamanho do byte
        arvore_i = arvore_size - 1, 
        n_i = arvore_i; // última posição da árvore
    for (int i = 0; i < size_f_arq;) {
      if(b_size == 0) { 
        byte = readArq.get();
        if(readArq.eof()) break;
        b_size = 8; // tamanho do byte 
      }
      n_i = buscar_caractere(arvore, byte, b_size, n_i); // busca o caractere na árvore pelo código
      if(arvore[n_i].left == -1) { // se for -1 então é folha
        writeArq.put(arvore[n_i].caractere); // escreve o caractere no arquivo descompactado
        n_i = arvore_i;
        i++;  
      }
    }
  } else if (size_arq == 5) {
      int n;
      ler_bytes(readArq, (char*)&n, sizeof(n));

      char c;
      ler_bytes(readArq, (char*)&c, sizeof(c));

      for (int i = 0; i < n; i++)
        writeArq.put(c);
  }
}

int main(int argc, char const *argv[]) {
  ifstream readArq; 
  ofstream writeArq; 
  
  if(argc > 3) {
    readArq.open(argv[2], ifstream::in);
    if(!readArq.is_open()) {
      cout << "Falha ao abrir o arquivo!" << endl;
      return 1;
    }

    writeArq.open(argv[3], ifstream::out);
    if(!writeArq.is_open()) {
      cout << "Falha ao abrir o arquivo!" << endl;
      return 1;
    }

    if(strcmp(argv[1], "-c") == 0) { 
      comprimir(readArq, writeArq);
    } else if (strcmp(argv[1], "-d") == 0) {
      descomprimir(readArq, writeArq);
    }

    readArq.close();
    readArq.close();
  }
  return 0;
}