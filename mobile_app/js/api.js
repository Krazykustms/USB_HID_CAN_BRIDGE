/**
 * myepicEFI API Communication Module
 * Handles all communication with ESP32 web API
 */

class EPICAPI {
    constructor(baseUrl = 'http://192.168.4.1') {
        this.baseUrl = baseUrl;
        this.updateInterval = 500; // Update every 500ms
        this.dataInterval = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
    }
    
    /**
     * Fetch real-time data
     */
    async getData() {
        try {
            const response = await fetch(`${this.baseUrl}/data`, {
                method: 'GET',
                headers: {
                    'Accept': 'application/json',
                },
                cache: 'no-cache'
            });
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const data = await response.json();
            this.reconnectAttempts = 0; // Reset on success
            return data;
        } catch (error) {
            console.error('Error fetching data:', error);
            this.reconnectAttempts++;
            throw error;
        }
    }
    
    /**
     * Get system health metrics
     */
    async getHealth() {
        try {
            const response = await fetch(`${this.baseUrl}/health`, {
                method: 'GET',
                headers: {
                    'Accept': 'application/json',
                },
                cache: 'no-cache'
            });
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error('Error fetching health:', error);
            throw error;
        }
    }
    
    /**
     * Get current configuration
     */
    async getConfig() {
        try {
            const response = await fetch(`${this.baseUrl}/config`, {
                method: 'GET',
                headers: {
                    'Accept': 'application/json',
                }
            });
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error('Error fetching config:', error);
            throw error;
        }
    }
    
    /**
     * Save configuration
     */
    async saveConfig(config) {
        try {
            const response = await fetch(`${this.baseUrl}/config/save`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json',
                },
                body: JSON.stringify(config)
            });
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error('Error saving config:', error);
            throw error;
        }
    }
    
    /**
     * Start periodic data updates
     */
    startUpdates(callback) {
        if (this.dataInterval) {
            this.stopUpdates();
        }
        
        const update = async () => {
            try {
                const data = await this.getData();
                callback(data);
            } catch (error) {
                callback(null, error);
            }
        };
        
        // Initial update
        update();
        
        // Periodic updates
        this.dataInterval = setInterval(update, this.updateInterval);
    }
    
    /**
     * Stop periodic updates
     */
    stopUpdates() {
        if (this.dataInterval) {
            clearInterval(this.dataInterval);
            this.dataInterval = null;
        }
    }
    
    /**
     * Check connection status
     */
    async checkConnection() {
        try {
            await this.getHealth();
            return true;
        } catch (error) {
            return false;
        }
    }
}

// Export singleton instance
const api = new EPICAPI();

