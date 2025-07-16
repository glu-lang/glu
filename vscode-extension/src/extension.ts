import * as vscode from 'vscode';
import * as path from 'path';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: vscode.ExtensionContext) {
    console.log('Glu Language Support extension is now active!');
    
    // Check if language server is enabled
    const config = vscode.workspace.getConfiguration('glu');
    const isEnabled = config.get('languageServer.enabled', true);
    
    if (!isEnabled) {
        console.log('Glu Language Server is disabled in configuration');
        return;
    }
    
    // Get the path to the language server
    let serverPath = config.get('languageServer.path', '');
    
    if (!serverPath) {
        // Try to find glulsp in common locations
        const possiblePaths = [
            path.join(context.extensionPath, '..', '..', 'build', 'tools', 'glulsp', 'sources', 'glulsp'),
            path.join(context.extensionPath, 'bin', 'glulsp'),
            'glulsp' // Try global PATH
        ];
        
        for (const possiblePath of possiblePaths) {
            try {
                require('fs').accessSync(possiblePath, require('fs').constants.F_OK);
                serverPath = possiblePath;
                break;
            } catch (e) {
                // Continue searching
            }
        }
    }
    
    if (!serverPath) {
        vscode.window.showWarningMessage(
            'Glu Language Server not found. Please set the path in settings or ensure glulsp is in your PATH.'
        );
        return;
    }
    
    // Server options
    const serverOptions: ServerOptions = {
        run: { command: serverPath, transport: TransportKind.stdio },
        debug: { command: serverPath, transport: TransportKind.stdio }
    };
    
    // Client options
    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'glu' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/.clientrc')
        }
    };
    
    // Create and start the language client
    client = new LanguageClient(
        'gluLanguageServer',
        'Glu Language Server',
        serverOptions,
        clientOptions
    );
    
    // Start the client
    client.start();
    
    console.log('Glu Language Server started with path:', serverPath);
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}