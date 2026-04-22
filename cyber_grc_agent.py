import json
import os
import sys
from pathlib import Path

import pandas as pd
from langchain_core.prompts import PromptTemplate
from langchain_core.output_parsers import StrOutputParser
from langchain_google_genai import ChatGoogleGenerativeAI
from dotenv import load_dotenv


def main() -> None:
    load_dotenv()

    csv_path = Path("data/alerts_filtered.csv")

    if not csv_path.exists():
        print(f"Erreur: fichier introuvable -> {csv_path}")
        sys.exit(1)

    try:
        df = pd.read_csv(csv_path)
    except Exception as exc:
        print(f"Erreur lors de la lecture du CSV: {exc}")
        sys.exit(1)

    if df.empty:
        print("Erreur: le fichier CSV est vide, aucune analyse possible.")
        sys.exit(1)

    sample_df = df.head(5)
    logs_sample = sample_df.to_dict(orient="records")
    logs_json = json.dumps(logs_sample, ensure_ascii=False)

    api_key = os.getenv("GOOGLE_API_KEY", "").strip()
    if not api_key or api_key == "YOUR_API_KEY_HERE":
        print("Erreur: GOOGLE_API_KEY absent. Ajoute la clé dans le fichier .env.")
        sys.exit(1)

    model_name = os.getenv("GEMINI_MODEL", "models/gemini-flash-latest")
    llm = ChatGoogleGenerativeAI(
        model=model_name,
        temperature=0,
    )

    prompt = PromptTemplate(
        input_variables=["logs"],
        template=(
            "Tu es un analyste senior en Gouvernance Cyber. "
            "Traduis ces logs techniques bruts en alertes opérationnelles. "
            "Retourne UNIQUEMENT un objet JSON valide contenant une liste nommée \"alertes\". "
            "Chaque alerte doit avoir les clés : \"ip_source\", \"niveau_criticite\" "
            "(Faible, Moyen, Critique), \"type_menace\", et \"action_recommandee\".\n\n"
            "Logs échantillonnés:\n{logs}"
        ),
    )

    chain = prompt | llm | StrOutputParser()

    try:
        result = chain.invoke({"logs": logs_json})
    except Exception as exc:
        print(f"Erreur pendant l'appel LLM: {exc}")
        sys.exit(1)

    print("=== RAPPORT DE GOUVERNANCE TEMPS RÉEL ===")
    print(result)


if __name__ == "__main__":
    main()
