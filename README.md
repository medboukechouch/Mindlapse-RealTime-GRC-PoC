

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
