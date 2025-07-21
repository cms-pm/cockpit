"""
PyOCD Manager for Oracle Bootloader Testing

Handles hardware reset and flash backup/restore operations via pyOCD.
Provides clean hardware state management for Oracle testing scenarios.
"""

import time
import logging
from typing import Optional, List
from pyocd.core.helpers import ConnectHelper
from pyocd.core.target import Target
from pyocd.core.session import Session

# Bootloader flash layout constants
BOOTLOADER_TEST_PAGE_ADDR = 0x0801F800  # Page 63 - last bytecode page
BOOTLOADER_FLASH_PAGE_SIZE = 2048        # STM32G431CB page size

logger = logging.getLogger(__name__)

class PyOCDManager:
    """
    Manages pyOCD connection for hardware reset and memory operations.
    Provides flash backup/restore safety net for Oracle testing.
    """
    
    def __init__(self):
        self.session: Optional[Session] = None
        self.target: Optional[Target] = None
        self.connected = False
        
    def connect(self) -> bool:
        """
        Connect to STM32G431CB target via pyOCD.
        Returns True if connection successful.
        """
        try:
            # Connect using same pattern as workspace test runner
            self.session = ConnectHelper.session_with_chosen_probe(
                target_override="cortex_m",
                connect_timeout=5  # 5 second timeout
            )
            self.target = self.session.target
            
            # Verify connection by reading PC register
            pc = self.target.read_core_register('pc')
            logger.info(f"Connected to STM32G431CB (PC: 0x{pc:08X})")
            
            self.connected = True
            return True
            
        except Exception as e:
            logger.error(f"Failed to connect via pyOCD: {e}")
            self.connected = False
            return False
    
    def disconnect(self):
        """Clean disconnect from target."""
        if self.session:
            try:
                self.session.close()
            except Exception as e:
                logger.warning(f"Error during disconnect: {e}")
        
        self.session = None
        self.target = None
        self.connected = False
        logger.info("Disconnected from target")
    
    def reset_target(self) -> bool:
        """
        Perform full halt/reset cycle for clean bootloader startup.
        Returns True if reset successful.
        """
        if not self.connected:
            logger.error("Not connected to target")
            return False
            
        try:
            # Halt target first
            self.target.halt()
            time.sleep(0.1)
            
            # Perform reset without halt
            self.target.reset(halt=False)
            time.sleep(0.1)
            
            # Resume execution - bootloader will start
            self.target.resume()
            
            logger.info("Target reset complete - bootloader starting")
            return True
            
        except Exception as e:
            logger.error(f"Reset failed: {e}")
            return False
    
    def backup_flash_page(self, address: int = BOOTLOADER_TEST_PAGE_ADDR) -> Optional[List[int]]:
        """
        Backup flash page contents for safety net.
        
        Args:
            address: Flash page starting address
            
        Returns:
            List of bytes if successful, None if failed
        """
        if not self.connected:
            logger.error("Not connected to target")
            return None
            
        try:
            # Read entire flash page (2KB)
            data = self.target.read_memory_block8(address, BOOTLOADER_FLASH_PAGE_SIZE)
            logger.info(f"Flash page backup complete: 0x{address:08X} ({len(data)} bytes)")
            return data
            
        except Exception as e:
            logger.error(f"Flash backup failed: {e}")
            return None
    
    def restore_flash_page(self, address: int, backup_data: List[int]) -> bool:
        """
        Restore flash page from backup data.
        
        Args:
            address: Flash page starting address
            backup_data: Previously backed up flash data
            
        Returns:
            True if restore successful
        """
        if not self.connected:
            logger.error("Not connected to target")
            return False
            
        if not backup_data or len(backup_data) != BOOTLOADER_FLASH_PAGE_SIZE:
            logger.error("Invalid backup data for restore")
            return False
            
        try:
            # Erase flash page first
            self.target.memory_map.get_boot_memory().erase_page(address)
            time.sleep(0.1)  # Allow erase to complete
            
            # Write backup data
            self.target.write_memory_block8(address, backup_data)
            
            # Verify write
            verify_data = self.target.read_memory_block8(address, BOOTLOADER_FLASH_PAGE_SIZE)
            if verify_data == backup_data:
                logger.info(f"Flash page restore complete: 0x{address:08X}")
                return True
            else:
                logger.error("Flash restore verification failed")
                return False
                
        except Exception as e:
            logger.error(f"Flash restore failed: {e}")
            return False
    
    def verify_bootloader_responsiveness(self) -> bool:
        """
        Basic check that bootloader is running and responsive.
        This is a placeholder for more sophisticated health checks.
        
        Returns:
            True if bootloader appears to be running
        """
        if not self.connected:
            return False
            
        try:
            # Read stack pointer and program counter as basic health check
            sp = self.target.read32(0x20000000)  # Initial stack pointer
            pc = self.target.read_core_register('pc')
            
            # Basic sanity checks
            if 0x20000000 <= sp <= 0x20008000 and 0x08000000 <= pc <= 0x08020000:
                logger.debug(f"Bootloader health check: SP=0x{sp:08X}, PC=0x{pc:08X}")
                return True
            else:
                logger.warning(f"Bootloader health check failed: SP=0x{sp:08X}, PC=0x{pc:08X}")
                return False
                
        except Exception as e:
            logger.error(f"Bootloader health check error: {e}")
            return False

class FlashSafety:
    """
    Flash backup/restore safety net for Oracle testing.
    Ensures clean state recovery after any scenario execution.
    """
    
    def __init__(self, pyocd_manager: PyOCDManager):
        self.pyocd = pyocd_manager
        self.backup_stack = []  # Stack of (address, data) tuples
    
    def create_backup(self, address: int = BOOTLOADER_TEST_PAGE_ADDR) -> Optional[int]:
        """
        Create flash page backup and return backup ID.
        
        Args:
            address: Flash address to backup
            
        Returns:
            Backup ID if successful, None if failed
        """
        backup_data = self.pyocd.backup_flash_page(address)
        if backup_data is not None:
            self.backup_stack.append((address, backup_data))
            backup_id = len(self.backup_stack) - 1
            logger.info(f"Created flash backup ID {backup_id} for address 0x{address:08X}")
            return backup_id
        return None
    
    def restore_backup(self, backup_id: Optional[int] = None) -> bool:
        """
        Restore flash from backup.
        
        Args:
            backup_id: Specific backup to restore, or most recent if None
            
        Returns:
            True if restore successful
        """
        if not self.backup_stack:
            logger.error("No backups available to restore")
            return False
            
        if backup_id is None:
            backup_id = len(self.backup_stack) - 1
            
        if backup_id < 0 or backup_id >= len(self.backup_stack):
            logger.error(f"Invalid backup ID: {backup_id}")
            return False
            
        address, backup_data = self.backup_stack[backup_id]
        success = self.pyocd.restore_flash_page(address, backup_data)
        
        if success:
            logger.info(f"Restored flash backup ID {backup_id}")
        
        return success
    
    def clear_backups(self):
        """Clear all backup data."""
        self.backup_stack.clear()
        logger.info("Cleared all flash backups")