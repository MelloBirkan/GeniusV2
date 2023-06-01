#include <Arduino.h> // Biblioteca principal do Arduino
#include <Wire.h> // Biblioteca para comunicação I2C
#include <Adafruit_SSD1306.h> // Biblioteca para controlar o display OLED
#include <EEPROM.h> // Biblioteca para manipulação da memória EEPROM

#define SCREEN_WIDTH 128 // Define a largura do display
#define SCREEN_HEIGHT 32 // Define a altura do display
#define OLED_RESET     -1 // Define o pino de reset para o display OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Cria um objeto 'display'

#define LED_RED 2 // Define o pino para o LED vermelho
#define LED_BLUE 3 // Define o pino para o LED azul
#define LED_YELLOW 4 // Define o pino para o LED amarelo
#define LED_GREEN 5 // Define o pino para o LED verde
#define INDEFINE -1 // Define o valor para estado indefinido

#define BUTTON_RED 8 // Define o pino para o botão vermelho
#define BUTTON_BLUE 9 // Define o pino para o botão azul
#define BUTTON_YELLOW 10 // Define o pino para o botão amarelo
#define BUTTON_GREEN 11 // Define o pino para o botão verde

#define BUZZER1  12 // Define o pino para o buzzer 1
#define BUZZER2  13 // Define o pino para o buzzer 2

#define N_LEDS 4 // Define a quantidade de LEDs
#define N_BUTTONS 4 // Define a quantidade de botões

#define S1 1000 // Define uma constante para um segundo (em ms)
#define S05 500 // Define uma constante para meio segundo (em ms)
#define SIZE_OF_SEQUENCE 2500 // Define o tamanho da sequência

const int LED_PINS[N_LEDS] = {LED_RED, LED_YELLOW, LED_BLUE, LED_GREEN}; // Array que armazena os pinos dos LEDs
const int BUTTON_PINS[N_BUTTONS] = {BUTTON_RED, BUTTON_BLUE, BUTTON_YELLOW, BUTTON_GREEN}; // Array que armazena os pinos dos botões
const int BUZZER_PINS[2] = {BUZZER1, BUZZER2}; // Array que armazena os pinos dos buzzers

enum States { // Define um enumerado para os estados do jogo
    READY_FOR_NEXT, // Pronto para a próxima rodada
    USER_PLAYING, // Usuário jogando
    GAME_FINISHED, // Jogo terminado
    GAME_OVER // Fim de jogo
};

int vectorLeds[SIZE_OF_SEQUENCE]; // Array para armazenar a sequência dos LEDs
int rounds = 0; // Variável para armazenar a rodada atual
int rodadaFinal = 0; // Variável para armazenar a última rodada jogada
int ledsAnswered = 0; // Variável para armazenar a quantidade de LEDs respondidos corretamente
int record = 0; // Variável para armazenar o recorde atual

void setup(){ // Função de configuração inicial

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Inicia o display
        Serial.println(F("SSD1306 allocation failed")); // Se houver falha, imprime uma mensagem
        for(;;); // Fica em loop infinito se houver falha
    }

    EEPROM.get(0, record); // Lê o valor do recorde armazenado na memória EEPROM
    display.clearDisplay(); // Limpa o display
    display.setTextSize(3); // Define o tamanho do texto
    display.setTextColor(SSD1306_WHITE); // Define a cor do texto
    display.setCursor(0,0); // Posiciona o cursor no início do display
    display.println("Genius"); // Imprime "Genius" no display
    display.display(); // Atualiza o display com as informações configuradas
    Serial.begin(9600); // Inicia a comunicação serial com uma taxa de transmissão de 9600 bits por segundo
    preSetupLedsNButtons(); // Chama a função que configura os LEDs e botões
    startGame(); // Chama a função que inicia o jogo
}

void preSetupLedsNButtons(){ // Função para configurar os LEDs e botões
    for(int i = 0; i < N_LEDS; i++){ // Para cada LED
        pinMode(LED_PINS[i], OUTPUT); // Define o pino como saída
    }

    for(int i = 0; i < N_BUTTONS; i++){ // Para cada botão
        pinMode(BUTTON_PINS[i], INPUT_PULLUP); // Define o pino como entrada com pull-up
    }

    for(int i = 0; i < 2; i++){ // Para cada buzzer
        pinMode(BUZZER_PINS[i], OUTPUT); // Define o pino como saída
    }
}

void startGame(){ // Função para iniciar o jogo

    int game = analogRead(0); // Lê o valor do pino analógico 0
    randomSeed(game); // Usa esse valor como semente para a geração de números aleatórios

    delay(1000); // Espera 1 segundo

    for(int i = 0; i < SIZE_OF_SEQUENCE; ++i) { // Para cada posição na sequência
        vectorLeds[i] = randomColor(); // Gera uma cor aleatória e armazena na posição correspondente
    }
}

int randomColor(){ // Função para gerar uma cor aleatória
    int randomC = random(LED_RED, LED_GREEN + 1); // Gera um número aleatório entre LED_RED e LED_GREEN + 1
    return randomC; // Retorna o número aleatório gerado
}

void loop(){ // Função principal que é executada continuamente

    switch (stateRightNow()) { // Avalia o estado atual do jogo
        case READY_FOR_NEXT: // Se estiver pronto para a próxima rodada
            Serial.println("Pronto para proxima rodada"); // Imprime uma mensagem na serial
            nextRound(); // Chama a função para iniciar a próxima rodada
            break;

        case USER_PLAYING: // Se o usuário estiver jogando
            userAnswer(); // Chama a função para processar a resposta do usuário
            //Serial.println("Esperando resposta do usuario"); // Imprime uma mensagem na serial
            break;

        case GAME_FINISHED: // Se o jogo tiver terminado
            congratulations(); // Chama a função para apresentar a mensagem de parabéns
            Serial.println("jogo finalizado com sucesso"); // Imprime uma mensagem na serial
            break;

        case GAME_OVER: // Se o jogo acabou
            fail(); // Chama a função para apresentar a mensagem de fim de jogo
            break;
    }
}

void playSound(int frequencia) { // Função que toca um som em um determinado tom
    tone(BUZZER2, frequencia, 100); // Usa a função tone para gerar um tom no buzzer
}


void ledSound(int led) { // Função que aciona um som associado a um led
    switch (led) { // Verifica qual é o led
        case LED_GREEN: // Se for o verde
            playSound(2000); // Toca um som de 2000Hz
            break;
        case LED_YELLOW: // Se for o amarelo
            playSound(2200); // Toca um som de 2200Hz
            break;
        case LED_RED: // Se for o vermelho
            playSound(2400); // Toca um som de 2400Hz
            break;
        case LED_BLUE: // Se for o azul
            playSound(2500); // Toca um som de 2500Hz
            break;
    }
}


void userAnswer() { // Função para processar a resposta do usuário
    int answer = checkButtonState(); // Verifica o estado dos botões
    if(answer == INDEFINE) return; // Se não houver resposta, retorna
    if(answer == vectorLeds[ledsAnswered]) ledsAnswered++; // Se a resposta for correta, incrementa a contagem de respostas corretas
    else{
        Serial.println("Reposta errada!"); // Se a resposta for errada, imprime uma mensagem na serial
        rounds = SIZE_OF_SEQUENCE + 2; // Define rounds para um valor que indica o fim do jogo
    }
}


void nextRound() { // Função para iniciar a próxima rodada
    rodadaFinal = rounds; // Guarda a rodada atual como a última rodada
    checkAndUpdateHighScore(); // Verifica e atualiza o recorde
    rounds++; // Incrementa a contagem de rodadas
    ledsAnswered = 0; // Reseta a contagem de respostas corretas

    // Configura o display
    display.clearDisplay(); // Limpa o display
    display.setTextSize(2); // Define o tamanho do texto
    display.setTextColor(SSD1306_WHITE); // Define a cor do texto

    // Define o texto para o display
    display.setCursor(0,0); // Posiciona o cursor no início do display
    display.println("Rodada: " + String(rounds)); // Imprime a rodada atual
    display.setTextSize(1);
    display.println("Record: " + String(record)); // Imprime o recorde

    // Atualiza o display
    display.display();

    if(rounds <= SIZE_OF_SEQUENCE) lightUpLedRounds(); // Se ainda não chegou no fim do jogo, acende os leds para a próxima rodada
}


int stateRightNow(){ // Função para verificar o estado atual do jogo
    if(rounds <= SIZE_OF_SEQUENCE) { // Se ainda não atingimos o limite de rodadas
        if (ledsAnswered == rounds) return READY_FOR_NEXT; // Se o usuário respondeu corretamente todas as perguntas da rodada atual, está pronto para a próxima
        else return USER_PLAYING; // Se ainda não respondeu todas, o usuário está jogando
    } else if(rounds == SIZE_OF_SEQUENCE + 1) return GAME_FINISHED; // Se atingimos o limite de rodadas, o jogo terminou
    else return GAME_OVER; // Se passamos do limite de rodadas, o jogo acabou
}


void lightUpLedRounds() { // Função para acender os leds da rodada atual

    for(int i = 0; i < rounds; ++i){ // Para cada rodada até a atual
        lightUp(vectorLeds[i]); // Acende o led correspondente
    }
}


int checkButtonState() { // Função para verificar o estado dos botões

    if(digitalRead(BUTTON_RED) == LOW) return lightUp(LED_RED); // Se o botão vermelho está pressionado, acende o led vermelho
    if(digitalRead(BUTTON_BLUE) == LOW) return lightUp(LED_BLUE); // Se o botão azul está pressionado, acende o led azul
    if(digitalRead(BUTTON_YELLOW) == LOW) return lightUp(LED_YELLOW); // Se o botão amarelo está pressionado, acende o led amarelo
    if(digitalRead(BUTTON_GREEN) == LOW) return lightUp(LED_GREEN); // Se o botão verde está pressionado, acende o led verde

    return INDEFINE; // Se nenhum botão está pressionado, retorna INDEFINE
}


void congratulations() { // Função para exibir a mensagem de parabéns

    lightUp(LED_RED); // Acende o LED vermelho
    lightUp(LED_BLUE); // Acende o LED azul
    lightUp(LED_YELLOW); // Acende o LED amarelo
    lightUp(LED_GREEN); // Acende o LED verde
    delay(S05); // Espera meio segundo
}


void fail() { // Função para exibir a mensagem de falha

    checkAndUpdateHighScore(); // Verifica e atualiza o recorde

    // Configura o display
    display.clearDisplay(); // Limpa o display
    display.setTextSize(2); // Define o tamanho do texto
    display.setTextColor(SSD1306_WHITE); // Define a cor do texto

    // Define o texto para o display
    display.setCursor(0,0); // Posiciona o cursor no início do display
    display.println("Game Over!"); // Imprime "Game Over"
    display.setTextSize(1);
    display.println("Rodada Final: " + String(rodadaFinal)); // Imprime a rodada final
    display.println("Record: " + String(record)); // Imprime o recorde

    // Atualiza o display
    display.display();
    for(int i = 0; i < 2; ++i){
        digitalWrite(LED_RED, HIGH); // Acende o LED vermelho
        digitalWrite(LED_BLUE, HIGH); // Acende o LED azul
        digitalWrite(LED_YELLOW, HIGH); // Acende o LED amarelo
        digitalWrite(LED_GREEN, HIGH); // Acende o LED verde
        delay(S1); // Espera um segundo

        for(int i = 0; i<10000; i++) {
            playSound(i); // Toca um som
        }

        digitalWrite(LED_RED, LOW); // Apaga o LED vermelho
        digitalWrite(LED_BLUE, LOW); // Apaga o LED azul
        digitalWrite(LED_YELLOW, LOW); // Apaga o LED amarelo
        digitalWrite(LED_GREEN, LOW); // Apaga o LED verde
        delay(S05); // Espera meio segundo
    }


    delay(1700); // Espera 1,7 segundos
    restartGame(); // Reinicia o jogo
}

int lightUp(int led){ // Função para acender o LED

    ledSound(led); // Toca o som correspondente ao LED

    digitalWrite(led, HIGH); // Acende o LED
    delay(S1); // Espera um segundo
    digitalWrite(led, LOW); // Apaga o LED
    delay(S05); // Espera meio segundo
    return led; // Retorna o LED que foi aceso
}

void restartGame() { // Função para reiniciar o jogo

    // Reset das variáveis do jogo
    for(int i = 0; i < SIZE_OF_SEQUENCE; ++i) {
        vectorLeds[i] = randomColor(); // Define uma nova sequência aleatória de cores
    }
    rounds = 0; // Reseta a contagem de rodadas
    ledsAnswered = 0; // Reseta a contagem de LEDs respondidos
}

void checkAndUpdateHighScore() { // Função para verificar e atualizar o recorde

    if (rodadaFinal > record) { // Se a rodada final foi maior que o recorde atual
        record = rodadaFinal; // Atualiza o recorde
        EEPROM.put(0, record); // Grava o novo recorde na memória EEPROM
    }
}


