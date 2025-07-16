"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = require("vscode");
const path = require("path");
const node_1 = require("vscode-languageclient/node");
let client;
function activate(context) {
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
            }
            catch (e) {
                // Continue searching
            }
        }
    }
    if (!serverPath) {
        vscode.window.showWarningMessage('Glu Language Server not found. Please set the path in settings or ensure glulsp is in your PATH.');
        return;
    }
    // Server options
    const serverOptions = {
        run: { command: serverPath, transport: node_1.TransportKind.stdio },
        debug: { command: serverPath, transport: node_1.TransportKind.stdio }
    };
    // Client options
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'glu' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/.clientrc')
        }
    };
    // Create and start the language client
    client = new node_1.LanguageClient('gluLanguageServer', 'Glu Language Server', serverOptions, clientOptions);
    // Start the client
    client.start();
    console.log('Glu Language Server started with path:', serverPath);
}
function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
//# sourceMappingURL=extension.js.map