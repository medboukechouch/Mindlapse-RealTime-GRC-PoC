

Proof of Concept d’architecture hybride conçu pour démontrer des compétences avancées en conception de systèmes complexes appliqués à la cybersécurité.

## L'Objectif
Les approches GRC traditionnelles restent trop lentes face au volume des événements de sécurité modernes. Ce PoC vise à transformer des logs techniques bruts en alertes métiers actionnables, quasi temps réel, avec un pipeline orienté performance et décision.

## L'Architecture Hybride

### 1) Le Moteur d'Ingestion (C++)
Parseur bas niveau optimisé I/O, conçu pour absorber et pré-filtrer rapidement des volumes importants de logs CSV avant analyse sémantique.

**Performance mesurée :**
Capable de pré-filtrer des logs bruts avec un débit de plus de 233 000 lignes/seconde (benchmark local : 500 000 lignes lues en 2139 ms).

### 2) Le Moteur d'Analyse (Python/LangChain)
Un moteur Python exploite LangChain et un LLM pour interpréter les anomalies filtrées, contextualiser le risque, puis générer des rapports JSON exploitables par les équipes GRC/SOC.

## Exemple visuel — Résultat final

```json
=== RAPPORT DE GOUVERNANCE TEMPS RÉEL ===
{
	"alertes": [
		{
			"ip_source": "195.241.151.7",
			"niveau_criticite": "Moyen",
			"type_menace": "Tentative d'escalade de privilèges (sudo)",
			"action_recommandee": "Vérifier la légitimité des tentatives de l'utilisateur 'achang' sur proxy-mow-01 et bloquer l'IP en cas de non-conformité."
		},
		{
			"ip_source": "164.218.94.112",
			"niveau_criticite": "Critique",
			"type_menace": "Suspicion de compromission de compte de service (www-data)",
			"action_recommandee": "Isoler immédiatement le serveur srv-ldn-02, bloquer l'IP source et auditer les processus lancés par l'utilisateur web."
		},
		{
			"ip_source": "45.250.247.54",
			"niveau_criticite": "Critique",
			"type_menace": "Tentative d'escalade de privilèges via compte technique (su)",
			"action_recommandee": "Analyser l'intégrité du serveur backup-prk-01 et vérifier la présence de shells inversés (reverse shells) liés à www-data."
		},
		{
			"ip_source": "186.144.249.195",
			"niveau_criticite": "Moyen",
			"type_menace": "Anomalie d'authentification système (cron/nginx)",
			"action_recommandee": "Inspecter les configurations de tâches planifiées sur srv-tok-03 pour détecter une persistance malveillante."
		},
		{
			"ip_source": "91.201.120.168",
			"niveau_criticite": "Faible",
			"type_menace": "Échec d'authentification sur compte sensible (admin)",
			"action_recommandee": "Placer l'IP sous surveillance accrue et s'assurer que l'authentification multi-facteurs (MFA) est active pour le compte admin."
		}
	]
}
```

## Comment exécuter le projet

### Prérequis
- C++17 + g++
- Python 3.10+
- Dépendances Python dans `python/requirements.txt`

### 1) Compiler le moteur C++ (optimisé)
```bash
g++ -O3 -std=c++17 cpp/log_parser.cpp -o cpp/log_parser.exe
```

### 2) Pré-filtrer les logs bruts
```bash
./cpp/log_parser.exe data/linux_auth_logs_labeled.csv data/alerts_filtered.csv
```

### 3) Installer les dépendances Python
```bash
pip install -r python/requirements.txt
```

### 4) Configurer la clé API LLM
Créer un fichier `.env` à la racine du projet :
```env
GOOGLE_API_KEY=YOUR_API_KEY_HERE
```

### 5) Lancer l'analyse GRC
```bash
python cyber_grc_agent.py
```

## Pourquoi cette architecture
Séparer l’I/O massif de l’inférence IA est un choix d’ingénierie structurant :

- **Le C++** absorbe la charge de parsing et de filtrage à très haut débit, avec faible overhead mémoire/CPU.
- **Le Python/LLM** reste focalisé sur la couche de valeur : interprétation, priorisation et formulation des actions.

Cette séparation des responsabilités améliore la **scalabilité** (chaque couche scale indépendamment), la **résilience** (dégradation contrôlée en cas de saturation LLM) et la **maintenabilité** (évolutions métier sans régression sur la couche performance).
