/*
 *  TCMPortMapperStub.m
 *  XBolo
 *
 *  See TCMPortMapperStub.h.
 */

#import "TCMPortMapperStub.h"

NSString * const TCMPortMapperDidFinishWorkNotification = @"TCMPortMapperDidFinishWorkNotification";

@implementation TCMPortMapping

+ (id)portMappingWithLocalPort:(int)aPrivatePort desiredExternalPort:(int)aPublicPort transportProtocol:(int)aTransportProtocol userInfo:(id)aUserInfo {
  TCMPortMapping *mapping = [[[self alloc] init] autorelease];
  mapping->_localPort = aPrivatePort;
  mapping->_externalPort = aPublicPort;
  mapping->_transportProtocol = aTransportProtocol;
  return mapping;
}

- (TCMPortMappingStatus)mappingStatus {
  return TCMPortMappingStatusUnmapped;
}

- (TCMPortMappingTransportProtocol)transportProtocol {
  return _transportProtocol;
}

- (int)externalPort {
  return _externalPort;
}

- (int)localPort {
  return _localPort;
}

@end

@implementation TCMPortMapper

+ (TCMPortMapper *)sharedInstance {
  static TCMPortMapper *sharedInstance = nil;
  if (sharedInstance == nil) {
    sharedInstance = [[TCMPortMapper alloc] init];
  }
  return sharedInstance;
}

- (id)init {
  if ((self = [super init]) != nil) {
    _portMappings = [[NSMutableSet alloc] init];
  }
  return self;
}

- (void)dealloc {
  [_portMappings release];
  [super dealloc];
}

- (void)setUserID:(NSString *)aUserID {
}

- (NSSet *)portMappings {
  return _portMappings;
}

- (void)addPortMapping:(TCMPortMapping *)aMapping {
  [_portMappings addObject:aMapping];
}

- (void)removePortMapping:(TCMPortMapping *)aMapping {
  [_portMappings removeObject:aMapping];
}

- (void)start {
  /* no port mapping available; report completion so waiting UI unblocks */
  [[NSNotificationCenter defaultCenter] performSelectorOnMainThread:@selector(postNotification:)
                                                         withObject:[NSNotification notificationWithName:TCMPortMapperDidFinishWorkNotification object:self]
                                                      waitUntilDone:NO];
}

@end
