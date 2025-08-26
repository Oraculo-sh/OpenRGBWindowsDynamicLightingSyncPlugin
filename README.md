# Windows Dynamic Lighting Sync Plugin para OpenRGB

Este plugin permite sincronizar dispositivos RGB do OpenRGB com o Windows Dynamic Lighting, proporcionando uma experiência de iluminação unificada no Windows 11.

## Características

- ✅ Interface simples para ativar/desativar sincronização
- ✅ Integração com Windows Dynamic Lighting API
- ✅ Botão de teste para verificar funcionalidade
- ✅ Sincronização em tempo real (100ms)
- ✅ Proteções condicionais para compilação multiplataforma

## Requisitos

- Windows 11 com Windows Dynamic Lighting habilitado
- OpenRGB instalado
- Dispositivos RGB compatíveis
- Visual C++ Redistributable 2019/2022

## Instalação

### Opção 1: Usar DLL Pré-compilada

1. Baixe o arquivo `WindowsDynamicLightingSyncSimple.dll` da pasta `release/`
2. Copie o arquivo para o diretório de plugins do OpenRGB:
   ```
   %APPDATA%\OpenRGB\plugins\
   ```
3. Reinicie o OpenRGB
4. O plugin aparecerá na aba "Plugins"

### Opção 2: Compilar do Código Fonte

#### Pré-requisitos
- Visual Studio 2019/2022 com C++
- Qt 5.15.0 (MSVC 2019 64-bit)
- Git

#### Passos

1. Clone o repositório:
   ```bash
   git clone https://github.com/seu-usuario/WindowsDynamicLightingSync.git
   cd WindowsDynamicLightingSync
   ```

2. Inicialize os submódulos:
   ```bash
   git submodule update --init --recursive
   ```

3. Execute o script de build:
   ```bash
   .\build_simple.bat
   ```

4. O arquivo DLL será criado em `release/WindowsDynamicLightingSyncSimple.dll`

## Como Usar

1. Abra o OpenRGB
2. Vá para a aba "Plugins"
3. Localize "Windows Dynamic Lighting Sync"
4. Marque a caixa "Ativar sincronização" para começar a sincronização
5. Use o botão "Testar cor (Azul)" para verificar se a funcionalidade está funcionando
6. O status será exibido na parte inferior do plugin

## Funcionalidades

### Interface do Plugin
- **Ativar sincronização**: Liga/desliga a sincronização automática
- **Testar cor (Azul)**: Aplica uma cor de teste para verificar a conectividade
- **Status**: Mostra o estado atual do plugin

### Sincronização
- Sincronização automática a cada 100ms quando ativada
- Detecção automática de dispositivos Windows Dynamic Lighting
- Aplicação de cores do OpenRGB para dispositivos Windows

## Estrutura do Projeto

```
WindowsDynamicLightingSync/
├── src/
│   ├── WindowsDynamicLightingSync_simple.h     # Header do plugin
│   ├── WindowsDynamicLightingSync_simple.cpp   # Implementação do plugin
│   └── WindowsDynamicLightingSync_simple.pro   # Arquivo de projeto Qt
├── release/
│   └── WindowsDynamicLightingSyncSimple.dll    # Plugin compilado
├── scripts/
│   └── build_simple.bat                        # Script de compilação
└── dependencies/
    └── OpenRGBSamplePlugin/                     # Dependências do OpenRGB
```

## Desenvolvimento

### Arquitetura

O plugin é baseado em Qt e utiliza:
- **Qt Widgets**: Para a interface do usuário
- **Windows Runtime (WinRT)**: Para integração com Windows Dynamic Lighting
- **QTimer**: Para sincronização periódica

### Principais Classes
- `WindowsDynamicLightingSyncSimple`: Classe principal do plugin
- Métodos principais:
  - `initializeWindowsLighting()`: Inicializa a API do Windows
  - `syncWithWindowsLighting()`: Executa a sincronização
  - `setupUI()`: Configura a interface do usuário

## Solução de Problemas

### Plugin não aparece no OpenRGB
- Verifique se o arquivo DLL está no diretório correto de plugins
- Certifique-se de que o OpenRGB foi reiniciado após a instalação
- Verifique se o Visual C++ Redistributable está instalado

### Sincronização não funciona
- Verifique se o Windows Dynamic Lighting está habilitado no Windows 11
- Certifique-se de que há dispositivos RGB compatíveis conectados
- Verifique os logs do OpenRGB para mensagens de erro

### Erro de compilação
- Verifique se o Qt 5.15.0 está instalado corretamente
- Certifique-se de que o Visual Studio 2019/2022 está configurado
- Execute `git submodule update --init --recursive` para garantir que as dependências estejam atualizadas

## Contribuição

Contribuições são bem-vindas! Por favor:

1. Faça um fork do projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## Agradecimentos

- Equipe do OpenRGB pelo framework de plugins
- Microsoft pela Windows Dynamic Lighting API
- Comunidade Qt pelo framework de interface

## Status do Desenvolvimento

- ✅ Implementação básica do plugin
- ✅ Interface de usuário funcional
- ✅ Integração com Windows Dynamic Lighting API
- ⏳ Detecção automática de dispositivos
- ⏳ Lógica avançada de sincronização
- ⏳ Tratamento robusto de erros
- ⏳ Sistema de logging detalhado
- ⏳ Testes unitários
- ⏳ Documentação completa