/*
 *  TCMPortMapperStub.h
 *  XBolo
 *
 *  Minimal stand-in for TCMPortMapper.framework (the bundled binary is
 *  ppc/i386 only and cannot link on modern macOS).  Declares just the API
 *  GSXBoloController uses.  start always reports failure, so hosting with
 *  UPnP checked shows the app's existing "UPnP Failed" alert; direct and
 *  LAN hosting are unaffected.
 */

#import <Foundation/Foundation.h>

extern NSString * const TCMPortMapperDidFinishWorkNotification;

typedef enum {
    TCMPortMappingStatusUnmapped = 0,
    TCMPortMappingStatusTrying   = 1,
    TCMPortMappingStatusMapped   = 2
} TCMPortMappingStatus;

typedef enum {
    TCMPortMappingTransportProtocolUDP  = 1,
    TCMPortMappingTransportProtocolTCP  = 2,
    TCMPortMappingTransportProtocolBoth = 3
} TCMPortMappingTransportProtocol;

@interface TCMPortMapping : NSObject {
  int _localPort;
  int _externalPort;
  TCMPortMappingTransportProtocol _transportProtocol;
}
+ (id)portMappingWithLocalPort:(int)aPrivatePort desiredExternalPort:(int)aPublicPort transportProtocol:(int)aTransportProtocol userInfo:(id)aUserInfo;
- (TCMPortMappingStatus)mappingStatus;
- (TCMPortMappingTransportProtocol)transportProtocol;
- (int)externalPort;
- (int)localPort;
@end

@interface TCMPortMapper : NSObject {
  NSMutableSet *_portMappings;
}
+ (TCMPortMapper *)sharedInstance;
- (void)setUserID:(NSString *)aUserID;
- (NSSet *)portMappings;
- (void)addPortMapping:(TCMPortMapping *)aMapping;
- (void)removePortMapping:(TCMPortMapping *)aMapping;
- (void)start;
@end
