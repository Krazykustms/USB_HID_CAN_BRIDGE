/**
 * UI Update Module
 * Handles all UI updates and animations
 */

class UIUpdater {
    constructor() {
        this.lastUpdateTime = new Date();
    }
    
    /**
     * Update gauge values
     */
    updateGauges(data) {
        if (!data) return;
        
        // Update RPM
        const rpmElement = document.getElementById('rpm-value');
        if (rpmElement) {
            this.animateValue(rpmElement, parseInt(rpmElement.textContent) || 0, data.rpm, 300);
        }
        
        // Update TPS
        const tpsElement = document.getElementById('tps-value');
        if (tpsElement) {
            this.animateValue(tpsElement, parseFloat(tpsElement.textContent) || 0, data.tps, 300, true);
        }
        
        // Update AFR
        const afrElement = document.getElementById('afr-value');
        if (afrElement) {
            this.animateValue(afrElement, parseFloat(afrElement.textContent) || 0, data.afr, 300, true);
        }
        
        // Update shift light
        this.updateShiftLight(data.shift_light || false);
    }
    
    /**
     * Animate value changes
     */
    animateValue(element, start, end, duration, decimals = false) {
        const startTime = performance.now();
        const difference = end - start;
        
        const animate = (currentTime) => {
            const elapsed = currentTime - startTime;
            const progress = Math.min(elapsed / duration, 1);
            
            // Easing function (ease-out)
            const easeOut = 1 - Math.pow(1 - progress, 3);
            const current = start + (difference * easeOut);
            
            if (decimals) {
                element.textContent = current.toFixed(1);
            } else {
                element.textContent = Math.round(current);
            }
            
            if (progress < 1) {
                requestAnimationFrame(animate);
            } else {
                element.textContent = decimals ? end.toFixed(1) : Math.round(end);
            }
        };
        
        requestAnimationFrame(animate);
    }
    
    /**
     * Update shift light indicator
     */
    updateShiftLight(active) {
        const indicator = document.getElementById('shift-light-indicator');
        if (indicator) {
            if (active) {
                indicator.classList.add('active');
                indicator.classList.remove('inactive');
            } else {
                indicator.classList.add('inactive');
                indicator.classList.remove('active');
            }
        }
    }
    
    /**
     * Update connection status
     */
    updateConnectionStatus(connected) {
        const statusElement = document.getElementById('connection-status');
        if (statusElement) {
            const dot = statusElement.querySelector('.status-dot');
            if (dot) {
                if (connected) {
                    dot.classList.add('connected');
                    dot.classList.remove('disconnected');
                    statusElement.title = 'Connected';
                } else {
                    dot.classList.add('disconnected');
                    dot.classList.remove('connected');
                    statusElement.title = 'Disconnected';
                }
            }
        }
    }
    
    /**
     * Update system status
     */
    updateSystemStatus(health) {
        const statusElement = document.getElementById('system-status');
        if (statusElement && health) {
            statusElement.textContent = health.system_state || 'UNKNOWN';
            statusElement.className = `stat-value status-${health.system_state?.toLowerCase() || 'unknown'}`;
        }
    }
    
    /**
     * Update uptime display
     */
    updateUptime(uptimeFormatted) {
        const uptimeElement = document.getElementById('uptime');
        if (uptimeElement && uptimeFormatted) {
            uptimeElement.textContent = uptimeFormatted;
        }
    }
    
    /**
     * Update last update time
     */
    updateLastUpdateTime() {
        const lastUpdateElement = document.getElementById('last-update');
        if (lastUpdateElement) {
            const now = new Date();
            const timeString = now.toLocaleTimeString();
            lastUpdateElement.textContent = `Last update: ${timeString}`;
        }
    }
    
    /**
     * Show error message
     */
    showError(message) {
        // Simple error notification (can be enhanced with toast notifications)
        console.error('Error:', message);
        // TODO: Implement toast notification system
    }
    
    /**
     * Show success message
     */
    showSuccess(message) {
        console.log('Success:', message);
        // TODO: Implement toast notification system
    }
}

// Export singleton instance
const uiUpdater = new UIUpdater();

