# Windows Dynamic Lighting Sync (OpenRGB Plugin)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE) [![Status](https://img.shields.io/badge/status-alpha-orange.svg)](#) [![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue.svg)](#) [![OpenRGB](https://img.shields.io/badge/OpenRGB-plugin-8A2BE2.svg)](#) [![Windows 11](https://img.shields.io/badge/Windows%2011-Compatible-00A4EF.svg)](#)

Sincronize a iluminação RGB do seu PC com o ecossistema nativo do Windows Dynamic Lighting, diretamente a partir do OpenRGB. Este projeto integra dispositivos RGB gerenciados pelo OpenRGB ao recurso de Iluminação Dinâmica do Windows, oferecendo uma experiência unificada, consistente e de baixa latência.

## Sobre o que é este projeto

Windows Dynamic Lighting Sync é um plugin para OpenRGB que expõe seus dispositivos RGB ao Windows 10/11 por meio do recurso Dynamic Lighting. Na prática, ele atua como uma ponte entre o OpenRGB e o Windows, permitindo que o sistema operacional aplique efeitos, cores e comportamentos de iluminação de forma integrada, sem perder a flexibilidade do OpenRGB.

## Principais funcionalidades

- Integração nativa com Windows Dynamic Lighting (Windows 10/11)
- Sincronização de cores em tempo real entre Windows e dispositivos do OpenRGB
- Mapeamento consistente de LEDs (cor e brilho) para resultados previsíveis
- Operação robusta com tratamento de erros e resiliência a desconexões
- Compatibilidade com múltiplas marcas e ecossistemas via OpenRGB

## Como funciona (visão geral)

- O plugin se conecta a um serviço local leve responsável por traduzir mensagens de protocolo entre o OpenRGB e o Windows Dynamic Lighting.
- Quando o Windows altera um efeito/cor, o plugin recebe a atualização e a aplica nos dispositivos presentes no OpenRGB.
- Quando o OpenRGB altera um estado, o plugin pode refletir essa mudança, mantendo consistência entre os dois lados quando aplicável.
- Toda comunicação é local (máquina do usuário), reduzindo latência e evitando dependência de rede.

## Escopo e objetivos

- Levar o ecossistema de dispositivos suportados pelo OpenRGB para o mundo do Dynamic Lighting do Windows.
- Preservar a liberdade de configuração do OpenRGB, enquanto habilita a compatibilidade com aplicativos e recursos do Windows.
- Fornecer experiência simples de uso, com mínimos passos para começar (sem detalhes técnicos neste documento).

## Compatibilidade

- Windows 10 (1903+) e Windows 11 com Dynamic Lighting habilitado.
- Dispositivos suportados pelo OpenRGB (marcas e controladores variados).
- OpenRGB com suporte a plugins habilitado.

## Status do projeto

- Versão: 0.1 alpha
- Foco atual: estabilidade do protocolo, cobertura de casos comuns e validações ponta a ponta.
- Melhorias planejadas: refinamentos de mapeamento de LEDs, métricas de telemetria opcionais, opções de personalização de sincronização e compatibilidade ampliada.

## Privacidade e segurança

- Toda a comunicação acontece localmente no seu computador.
- O projeto não coleta dados pessoais, nem transfere informações para a internet.
- Logs e diagnósticos (quando ativados) permanecem locais, destinados apenas à análise de problemas.

## Créditos e agradecimentos

- OpenRGB – pela base sólida de controle de iluminação multiplataforma.
- Comunidade OpenRGB – pelas referências, amostras e documentação.
- Microsoft – pelo recurso Windows Dynamic Lighting.

## Licença

Distribuído sob a licença MIT. Consulte o arquivo [LICENSE](LICENSE) para mais detalhes.
