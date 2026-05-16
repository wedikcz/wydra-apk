// Minimal Frida-compatible agent for embedded use
// This runs in the context of the target process when attached

'use strict';

rpc.exports = {
    readMemory: function(address, size) {
        try {
            if (size <= 0 || size > 0x10000000) return null;
            var data = Memory.readByteArray(ptr(address), size);
            if (!data) return null;
            return Array.prototype.slice.call(new Uint8Array(data));
        } catch(e) { return null; }
    },
    
    writeMemory: function(address, bytes) {
        try {
            Memory.writeByteArray(ptr(address), bytes);
            return true;
        } catch(e) { return false; }
    },
    
    listModules: function() {
        return Process.enumerateModules().map(function(m) {
            return {
                name: m.name,
                base: m.base.toString(),
                size: m.size,
                path: m.path
            };
        });
    },
    
    hookExport: function(moduleName, exportName, returnValue) {
        try {
            var addr = Module.findExportByName(moduleName, exportName);
            if (!addr) return false;
            
            Interceptor.attach(addr, {
                onLeave: function(retval) {
                    retval.replace(ptr(returnValue));
                }
            });
            return true;
        } catch(e) { return false; }
    },
    
    forceReturn: function(address, value) {
        try {
            Interceptor.attach(ptr(address), {
                onLeave: function(retval) {
                    retval.replace(ptr(value));
                }
            });
            return true;
        } catch(e) { return false; }
    },
    
    bypassAntiDebug: function() {
        try {
            // Hook ptrace
            var ptrace = Module.findExportByName(null, 'ptrace');
            if (ptrace) {
                Interceptor.attach(ptrace, {
                    onEnter: function(args) {
                        var request = args[0].toInt32();
                        if (request === 0 || request === 16 || request === 31) {
                            args[0] = ptr(-1);
                        }
                    }
                });
            }
            
            // Hook strstr
            var strstr = Module.findExportByName(null, 'strstr');
            if (strstr) {
                Interceptor.attach(strstr, {
                    onEnter: function(args) {
                        var needle = args[1].readCString();
                        if (needle) {
                            var blocked = ['frida', 'gdb', 'lldb', 'ptrace', 'debug', 'xposed', 'magisk'];
                            for (var i = 0; i < blocked.length; i++) {
                                if (needle.toLowerCase().indexOf(blocked[i]) >= 0) {
                                    args[0] = ptr(0);
                                    return;
                                }
                            }
                        }
                    }
                });
            }
            
            return true;
        } catch(e) { return false; }
    },
    
    suspendThreads: function() {
        try {
            var count = 0;
            Process.enumerateThreads().forEach(function(t) {
                if (t.id !== Process.id) {
                    t.suspend();
                    count++;
                }
            });
            return count;
        } catch(e) { return 0; }
    },
    
    resumeThreads: function() {
        try {
            var count = 0;
            Process.enumerateThreads().forEach(function(t) {
                if (t.id !== Process.id) {
                    t.resume();
                    count++;
                }
            });
            return count;
        } catch(e) { return 0; }
    },
    
    getProcessInfo: function() {
        return {
            pid: Process.id,
            arch: Process.arch,
            platform: Process.platform,
            pageSize: Process.pageSize,
            pointerSize: Process.pointerSize,
            debuggerAttached: Process.isDebuggerAttached()
        };
    },
    
    findExport: function(moduleName, exportName) {
        try {
            var addr = Module.findExportByName(moduleName, exportName);
            return addr ? addr.toString() : null;
        } catch(e) { return null; }
    }
};

console.log('[Embedded Frida Agent] Loaded successfully');
