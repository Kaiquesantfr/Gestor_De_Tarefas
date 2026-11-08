#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <limits>

using namespace std;

enum TipoAcao { ACAO_ADICIONAR = 1, ACAO_REMOVER = 2, ACAO_CONCLUIR = 3 };


// ESTRUTURA BASE - Tarefa (no da lista encadeada)

struct Tarefa {
    int     id;
    string  titulo;
    int     prioridade;   // 1=baixa, 2=media, 3=alta
    int     prazo;        // formato AAAAMMDD (facilita comparar datas)
    bool    concluida;
    Tarefa* prox;
    Tarefa(int i = 0, const string& t = "", int p = 1, int pz = 0, bool c = false)
        : id(i), titulo(t), prioridade(p), prazo(pz), concluida(c), prox(nullptr) {}
};


// FUNCOES AUXILIARES (entrada de dados e formatacao)  - R1

// Converte texto para minusculas (busca sem diferenciar maiusculas).
// Parametro: s (por valor, pois a copia e modificada). Retorno: texto minusculo.
string minusculas(string s) {
    for (char& c : s) c = tolower(static_cast<unsigned char>(c));
    return s;
}

// Encerra o programa se a entrada acabou (evita laco infinito em cin).
void verificarEntrada() {
    if (cin.eof()) {
        cout << "\nEntrada encerrada.\n";
        exit(0);
    }
}

// Le um inteiro validado dentro de [minimo, maximo].
// Parametros: mensagem exibida, limites. Retorno: valor valido digitado.
int lerInteiro(const string& mensagem, int minimo, int maximo) {
    int valor;
    while (true) {
        cout << mensagem;
        if (cin >> valor && valor >= minimo && valor <= maximo) return valor;
        verificarEntrada();
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Valor invalido! Digite um numero entre " << minimo
             << " e " << maximo << ".\n";
    }
}

// Le uma linha de texto nao vazia (aceita espacos).
// Parametro: mensagem exibida. Retorno: texto digitado.
string lerTexto(const string& mensagem) {
    string texto;
    do {
        cout << mensagem;
        getline(cin >> ws, texto);
        verificarEntrada();
    } while (texto.empty());
    return texto;
}

// Pergunta sim/nao ate receber resposta valida.
// Parametro: mensagem exibida. Retorno: true para sim, false para nao.
bool perguntaSimNao(const string& mensagem) {
    string resposta;
    while (true) {
        cout << mensagem;
        cin >> resposta;
        verificarEntrada();
        resposta = minusculas(resposta);
        if (resposta == "s" || resposta == "sim") return true;
        if (resposta == "n" || resposta == "nao") return false;
        cout << "Resposta invalida! Digite 's' para sim ou 'n' para nao.\n";
    }
}

// Transforma um prazo AAAAMMDD em texto DD/MM/AAAA.
string formatarPrazo(int prazo) {
    int dia = prazo % 100, mes = (prazo / 100) % 100, ano = prazo / 10000;
    string d = string(dia < 10 ? "0" : "") + to_string(dia);
    string m = string(mes < 10 ? "0" : "") + to_string(mes);
    return d + "/" + m + "/" + to_string(ano);
}

// Retorna o nome da prioridade (1, 2 ou 3).
string nomePrioridade(int prioridade) {
    if (prioridade == 3) return "Alta";
    if (prioridade == 2) return "Media";
    return "Baixa";
}

// Imprime uma tarefa em uma linha. Parametro: tarefa (referencia constante).
void imprimirTarefa(const Tarefa& t) {
    cout << "  [" << t.id << "] " << t.titulo
         << " | Prioridade: " << nomePrioridade(t.prioridade)
         << " | Prazo: " << formatarPrazo(t.prazo)
         << " | " << (t.concluida ? "[x] concluida" : "[ ] pendente") << "\n";
}

// R5 - LISTA ENCADEADA DE TAREFAS
struct ListaTarefas {
    Tarefa* head = nullptr;
    int proximoId = 1;

    // Insere uma tarefa no fim da lista. Retorna ponteiro para o novo no.
    Tarefa* inserirFim(int id, const string& titulo, int prioridade,
                       int prazo, bool concluida) {
        Tarefa* novo = new Tarefa(id, titulo, prioridade, prazo, concluida);
        if (head == nullptr) {
            head = novo;
            return novo;
        }
        Tarefa* atual = head;
        while (atual->prox != nullptr) atual = atual->prox;
        atual->prox = novo;
        return novo;
    }

    // Remove a tarefa de um id. Guarda uma copia em 'copia' (para o desfazer).
    // Retorno: true se removeu, false se o id nao existe.
    bool remover(int id, Tarefa& copia) {
        Tarefa* anterior = nullptr;
        Tarefa* atual = head;
        while (atual != nullptr && atual->id != id) {
            anterior = atual;
            atual = atual->prox;
        }
        if (atual == nullptr) return false;
        copia = *atual;
        copia.prox = nullptr;
        if (anterior == nullptr) head = atual->prox;
        else anterior->prox = atual->prox;
        delete atual;
        atual = nullptr;
        return true;
    }

    // Busca sequencial por id. Retorno: ponteiro para a tarefa ou nullptr.
    Tarefa* buscarPorId(int id) const {
        Tarefa* atual = head;
        while (atual != nullptr && atual->id != id) atual = atual->prox;
        return atual;
    }

    // R3 - Conta os nos de forma RECURSIVA. Complexidade O(n).
    int contarRecursivo(const Tarefa* no) const {
        if (no == nullptr) return 0;
        return 1 + contarRecursivo(no->prox);
    }

    // R3 - Conta as tarefas pendentes de forma RECURSIVA. Complexidade O(n).
    int contarPendentes(const Tarefa* no) const {
        if (no == nullptr) return 0;
        return (no->concluida ? 0 : 1) + contarPendentes(no->prox);
    }

    // Percorre a lista imprimindo todas as tarefas.
    void imprimir() const {
        for (Tarefa* atual = head; atual != nullptr; atual = atual->prox)
            imprimirTarefa(*atual);
    }

    bool isEmpty() const { return head == nullptr; }

    ~ListaTarefas() {
        while (head != nullptr) {
            Tarefa* t = head;
            head = head->prox;
            delete t;
            t = nullptr;
        }
    }
};

// R6 - PILHA DE ACOES (LIFO) - base do "desfazer"
// A ultima acao feita e a primeira a ser desfeita: LIFO natural.
struct NoAcao {
    int     tipo;      // TipoAcao
    Tarefa  tarefa;    // copia da tarefa afetada
    NoAcao* prox;
    NoAcao(int t, const Tarefa& tf, NoAcao* p) : tipo(t), tarefa(tf), prox(p) {}
};

struct PilhaAcoes {
    NoAcao* topo = nullptr;

    // Empilha uma acao realizada, com copia da tarefa afetada.
    void push(int tipo, const Tarefa& tarefa) {
        topo = new NoAcao(tipo, tarefa, topo);
        topo->tarefa.prox = nullptr;
    }

    // Desempilha a ultima acao. Preenche tipo e tarefa por referencia.
    // Retorno: false se a pilha estava vazia.
    bool pop(int& tipo, Tarefa& tarefa) {
        if (topo == nullptr) return false;
        NoAcao* temp = topo;
        tipo = temp->tipo;
        tarefa = temp->tarefa;
        topo = topo->prox;
        delete temp;
        temp = nullptr;
        return true;
    }

    bool isEmpty() const { return topo == nullptr; }

    ~PilhaAcoes() {
        int tipo;
        Tarefa descarte;
        while (pop(tipo, descarte)) {}
    }
};


struct Gestor {
    ListaTarefas lista;
    PilhaAcoes   pilha;
};

// OPERACOES DO MENU

// Cadastra UMA tarefa pedindo os dados ao usuario e registra no historico.
void cadastrarUmaTarefa(Gestor& g) {
    string titulo = lerTexto("Titulo da tarefa: ");
    int prioridade = lerInteiro("Prioridade (1=baixa, 2=media, 3=alta): ", 1, 3);
    cout << "Prazo de entrega:\n";
    int dia = lerInteiro("  Dia (1-31): ", 1, 31);
    int mes = lerInteiro("  Mes (1-12): ", 1, 12);
    int ano = lerInteiro("  Ano (2026-2100): ", 2026, 2100);
    int prazo = ano * 10000 + mes * 100 + dia;

    int id = g.lista.proximoId++;
    Tarefa* nova = g.lista.inserirFim(id, titulo, prioridade, prazo, false);
    g.pilha.push(ACAO_ADICIONAR, *nova);
    cout << "Tarefa [" << id << "] cadastrada!\n";
}

// Repete o cadastro enquanto o usuario quiser (mesmo estilo do hospital).
void cadastrarTarefas(Gestor& g) {
    cout << "\n=== CADASTRO DE TAREFAS ===\n";
    bool mais = true;
    do {
        cadastrarUmaTarefa(g);
        mais = perguntaSimNao("Tem mais tarefas? (s/n): ");
        cout << "---------------------------------------------------\n";
    } while (mais);
}

// Lista todas as tarefas e mostra totais (contagem recursiva).
void listarTarefas(const Gestor& g) {
    cout << "\n=== SUAS TAREFAS ===\n";
    if (g.lista.isEmpty()) {
        cout << "(nenhuma tarefa cadastrada)\n";
        return;
    }
    g.lista.imprimir();
    cout << "Total: " << g.lista.contarRecursivo(g.lista.head)
         << " | Pendentes: " << g.lista.contarPendentes(g.lista.head) << "\n";
}

// Marca uma tarefa como concluida e empilha a acao para poder desfazer.
void concluirTarefa(Gestor& g) {
    if (g.lista.isEmpty()) { cout << "(nenhuma tarefa cadastrada)\n"; return; }
    int id = lerInteiro("ID da tarefa a concluir: ", 1, 999999);
    Tarefa* t = g.lista.buscarPorId(id);
    if (t == nullptr)  { cout << "Tarefa nao encontrada.\n"; return; }
    if (t->concluida)  { cout << "Essa tarefa ja esta concluida.\n"; return; }
    t->concluida = true;
    g.pilha.push(ACAO_CONCLUIR, *t);
    cout << "Tarefa \"" << t->titulo << "\" concluida!\n";
}

// Remove uma tarefa (com confirmacao) e empilha a acao.
void removerTarefa(Gestor& g) {
    if (g.lista.isEmpty()) { cout << "(nenhuma tarefa cadastrada)\n"; return; }
    int id = lerInteiro("ID da tarefa a remover: ", 1, 999999);
    Tarefa* t = g.lista.buscarPorId(id);
    if (t == nullptr) { cout << "Tarefa nao encontrada.\n"; return; }
    imprimirTarefa(*t);
    if (!perguntaSimNao("Confirma a remocao? (s/n): ")) return;
    Tarefa copia;
    g.lista.remover(id, copia);
    g.pilha.push(ACAO_REMOVER, copia);
    cout << "Tarefa removida. Use 'Desfazer' se mudar de ideia.\n";
}

// Desfaz a ultima acao (desempilha e aplica o inverso).
void desfazerAcao(Gestor& g) {
    int tipo;
    Tarefa t;
    if (!g.pilha.pop(tipo, t)) {
        cout << "Nada para desfazer.\n";
        return;
    }
    if (tipo == ACAO_ADICIONAR) {
        Tarefa descarte;
        g.lista.remover(t.id, descarte);
        cout << "Desfeito: cadastro de \"" << t.titulo << "\".\n";
    } else if (tipo == ACAO_REMOVER) {
        g.lista.inserirFim(t.id, t.titulo, t.prioridade, t.prazo, t.concluida);
        cout << "Desfeito: remocao de \"" << t.titulo << "\" (tarefa restaurada).\n";
    } else {
        Tarefa* original = g.lista.buscarPorId(t.id);
        if (original != nullptr) original->concluida = false;
        cout << "Desfeito: conclusao de \"" << t.titulo << "\".\n";
    }
}

// Exibe o menu principal.
void mostrarMenu() {
    cout << "\n================= MENU =================\n"
         << " 1) Cadastrar tarefas\n"
         << " 2) Listar tarefas\n"
         << " 3) Concluir tarefa\n"
         << " 4) Remover tarefa\n"
         << " 5) Desfazer ultima acao (Ctrl+Z)\n"
         << " 0) Sair\n";
}

// Chama a operacao correspondente a opcao escolhida.
void executarOpcao(Gestor& g, int opcao) {
    switch (opcao) {
        case 1:  cadastrarTarefas(g);  break;
        case 2:  listarTarefas(g);     break;
        case 3:  concluirTarefa(g);    break;
        case 4:  removerTarefa(g);     break;
        case 5:  desfazerAcao(g);      break;
        default: break;
    }
}

int main() {
    Gestor gestor;
    cout << "  Gestor de Tarefas - Etapa 1 (lista e pilha)  \n";

    int opcao;
    do {
        mostrarMenu();
        opcao = lerInteiro("Escolha uma opcao: ", 0, 5);
        executarOpcao(gestor, opcao);
    } while (opcao != 0);

    cout << "\nAte logo!\n";
    return 0;
}