#include <alt_types.h> // Supposition pour alt_u32
#include <system.h>    // Habituellement requis pour les macros de NIOS II
#include <stdio.h>     // Pour alt_printf
#include <unistd.h>    // Pour usleep
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"


#define BUTTON_PRESS_DURATION 30000000 // 30 secondes en microsecondes

// Déclarations globales
volatile int edge_capture;
volatile int password_index = 0; // Index du chiffre actuel du mot de passe
volatile int password[4] = {0, 0, 0, 0}; // Mot de passe saisi par l'utilisateur
const int true_password[4] = {1, 2, 3, 4}; // Le vrai mot de passe à 4 chiffres
volatile int *edge_capture_ptr = NULL; // Exemple de déclaration globale
volatile int attempt_count = 0; // Compteur de tentatives de saisie
volatile int is_locked = 0; // Indicateur de verrouillage
const int max_attempts = 3; // Nombre maximal de tentatives
volatile alt_u32 lock_timer = 0; // Timer pour le verrouillage

volatile int is_setting_password = 0; // Un flag pour indiquer que nous sommes en mode de changement de mot de passe
volatile int new_password[4] = {0, 0, 0, 0};

volatile alt_u32 button_press_start_time = 0;
volatile int *key_ptr = (int *)KEY_BASE; // Pour le contrôle des 4 boutons
volatile int *hex_ptr = (int *)HEX_BASE; // Afficheurs 7 segments
volatile int *ledg_ptr = (int *)LEDG_BASE; // LED verte
volatile int *ledr_ptr = (int *)LEDR_BASE; // LED rouge

alt_u32 unlock_time;

// Mapping pour l'affichage 7 segments
char seven_seg[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

void display_password() {
    *hex_ptr = (((seven_seg[password[3]] << 24) & 0xFF000000) |
                ((seven_seg[password[2]] << 16) & 0xFF0000) |
                ((seven_seg[password[1]] << 8) & 0xFF00) |
                seven_seg[password[0]]);
}

// Fonction pour comparer le mot de passe saisi avec le mot de passe correct
int check_password() {
    for (int i = 0; i < 4; i++) {
        if (password[i] != true_password[i]) {
            return 0; // Mot de passe incorrect
        }
    }
    return 1; // Mot de passe correct
}

// Fonction pour afficher l'état du mot de passe sur les LEDs
void display_led_status(int is_correct) {
    if (is_correct) {
        *ledg_ptr = 0x1; // Allume la LED verte
        *ledr_ptr = 0x0; // Éteint la LED rouge
    } else {
        *ledg_ptr = 0x0; // Éteint la LED verte
        *ledr_ptr = 0x1; // Allume la LED rouge
    }
}

void handle_button_presses(void *context, alt_u32 id) {
    int press = *key_ptr;
    *key_ptr = 0x0; // Effacer les flags d'interruption

	if (press & 0x2) { // Bouton 1
        if (!button_press_start_time) { // Si le bouton vient d'être pressé
            button_press_start_time = get_current_time();
        }
    } else {
        if (button_press_start_time) { // Si le bouton a été relâché
            alt_u32 button_press_duration = get_current_time() - button_press_start_time;
            button_press_start_time = 0; // Réinitialiser pour la prochaine pression
            
            if (button_press_duration >= BUTTON_PRESS_DURATION) {
                // Code pour initier le changement de mot de passe
                start_password_change_routine();
            }
        }
    }

	if (is_locked){
		// Ici, On peut ajoutez un feedback pour indiquer que le système est verrouillé, si vous le souhaitez
		*ledr_ptr = 0x2;
		*key_ptr = 0xF;
		 return;
	}

    if (press & 0x1) { // Bouton 0
        password[password_index] = (password[password_index] + 1) % 10;
        display_password();
    }
    
    if (press & 0x2) { // Bouton 1
        password_index = (password_index + 1) % 4;
    }
    
    if (press & 0x4) { // Bouton 2
        if (password_index > 0) {
            password[--password_index] = 0;
            display_password();
        }
    }
    
    if (press & 0x8) { // Bouton 3
        if (check_password()) {
            display_led_status(1);
            attempt_count = 0; // Réinitialiser le compteur si le mot de passe est correct
        } else {
            attempt_count++; // Incrémenter le compteur de tentatives
            display_led_status(0);
            if (attempt_count >= max_attempts) {
                is_locked = 1; // Activer le verrouillage
                unlock_time = get_current_time() + (60 * 60 * 50e6);
                // Ici, On peut ajoutez un feedback pour indiquer que le système est verrouillé
                 *ledr_ptr = 0x2; //  fait clignoter la LED rouge
            }
        }
        // Réinitialise le mot de passe et l'index après la validation
        for (int i = 0; i < 4; i++) {
            password[i] = 0;
        }
        password_index = 0;
        display_password();
    }

    *key_ptr = 0xF; // Réactiver les interruptions des boutons
}

void init_button_interrupts() {
    void* edge_capture_ptr = (void*)&edge_capture;
    *key_ptr = 0x0; // Effacer les flags d'interruption
    *(key_ptr + 2) = 0xF; // Activer les interruptions pour tous les boutons
    alt_irq_register(KEY_IRQ, edge_capture_ptr, handle_button_presses);
}

void start_password_change_routine() {
    if(!is_locked){
		is_setting_password = 1;
		alt_irq_register(KEY_IRQ, edge_capture_ptr, handle_password_setting);
		password_index = 0; // Commencer à la première position du mot de passe
		for (int i = 0; i < 4; ++i) {
			new_password[i] = 0; // Réinitialiser le nouveau mot de passe
		}
		display_password(); // Afficher le nouveau mot de passe (initialement "0000")
	}
}

void handle_password_setting(void *context, alt_u32 id) {
    int press = *key_ptr;
    *key_ptr = 0x0; // Effacer les flags d'interruption

    if (press & 0x1) { // Bouton 0 : incrémenter le chiffre
        new_password[password_index] = (new_password[password_index] + 1) % 10;
        display_new_password();
    }

    if (press & 0x2) { // Bouton 1 : passer au chiffre suivant
        password_index = (password_index + 1) % 4;
    }

    if (press & 0x4) { // Bouton 2 : effacer le dernier chiffre
        if (password_index > 0) {
            new_password[--password_index] = 0;
            display_new_password();
        }
    }

    if (press & 0x8) { // Bouton 3 : valider le nouveau mot de passe
        for (int i = 0; i < 4; ++i) {
            password[i] = new_password[i]; // Copier le nouveau mot de passe
        }
        is_setting_password = 0; // Sortir du mode de changement de mot de passe
        display_password(); // Afficher le mot de passe (qui est maintenant le nouveau)
    }

    *key_ptr = 0xF; // Réactiver les interruptions des boutons
}

// Cette fonction est utilisée pour afficher le nouveau mot de passe lors du changement
void display_new_password() {
    *hex_ptr = (((seven_seg[new_password[3]] << 24) & 0xFF000000) |
                ((seven_seg[new_password[2]] << 16) & 0xFF0000) |
                ((seven_seg[new_password[1]] << 8) & 0xFF00) |
                seven_seg[new_password[0]]);
}
int main() {
    alt_printf("Début du programme de verrouillage...\n");
    
    // Initialisation des affichages et des interruptions
    display_password();
    init_button_interrupts();
    
	unsigned long unlock_time = 0;
    while (1) {
		if (is_setting_password) {
    // Utiliser 'handle_password_setting' à la place de 'handle_button_presses'
		}
        if (is_locked) {
            // Vérifier si une heure s'est écoulée
            if (get_current_time() >= unlock_time) {
                is_locked = 0; // Désactiver le verrouillage
                attempt_count = 0; // Réinitialiser le compteur de tentatives
                *ledr_ptr = 0x0; // Éteindre la LED rouge
            } else {
                // Clignotement de la LED rouge pour indiquer le verrouillage
                static int led_state = 0;
                *ledr_ptr = led_state; // Mettre à jour l'état de la LED rouge
                led_state = !led_state; // Inverser l'état pour la prochaine fois
                usleep(500000); // Attendre 500 ms pour le clignotement
            }
        } else {
            // Si le système n'est pas verrouillé, nous pourrions vouloir éteindre la LED rouge
            *ledr_ptr = 0x0;
        }
        
        // Vous pourriez avoir besoin d'une pause pour éviter une utilisation excessive du CPU
        usleep(1000); // Attendre 1 ms (ajustez selon les besoins)
    }


    return 0;
}
